# Lesson: panels stop going dark after 5 minutes on lewis

**Date:** 2026-09-18 (recurrence of 2026-09-16 / 2026-09-17 work)

## Symptom

The monitors on lewis stay lit indefinitely even though nobody has touched
the machine for hours. Expected behaviour: all panels blank after 5 minutes idle.

## Where the feature lives (not in this repo)

The blanking is NOT done by dwm, xset timers, or logind. It is a user service:

```
~/.config/systemd/user/idle-suspend.service   -> runs
~/scripts/idle-suspend.sh                     -> polls
~/.local/bin/xidle  (built from ~/scripts/xidle.c)
```

The script reads the X server idle counter every 30 s and runs
`xset dpms force off` once idle >= `IDLE_DPMS_SEC` (300 s).
`IDLE_SUSPEND_SEC=0` in the unit means panels-only mode: blank, never suspend,
because lewis is the fleet hub and a.o must stay reachable. Set 900 to restore
suspend.

Plain `xset dpms 0 0 300` does not work here: a browser wake lock suppresses the
server DPMS timer while the idle counter keeps growing.

## Root cause this time

The script only forced DPMS when `xset q` said "Monitor is On". After an xrandr
reconfigure (dock-monitor.sh hotplug at 13:58, or dwm-hotswap.sh reapplying the
layout) the panels light up again but the X server keeps reporting
"Monitor is Off". The guard therefore never fired again. Observed state:
idle 20 h, "Monitor is Off", all three panels lit.

## Fix applied

`idle-suspend.sh` now re-forces DPMS off on every poll while idle (idempotent
when the panels are already dark) and logs once per idle period, instead of
trusting the reported monitor state.

## Second finding (same evening, 2026-09-18)

The real reason nothing happened: lewis had TWO X servers. `:0` on tty1 was a
stale session from 2026-09-17; the live desktop was `:1` on tty4 (started by a
midnight `startx`). The unit hardcoded `DISPLAY=:0`, so every `xset dpms force
off` went to the server that did not own the VT. The kernel ignores DPMS from
an inactive VT, while `xset q` on that server still says "Monitor is Off".
Check with:

```bash
cat /sys/class/tty/tty0/active   # which VT is live
pgrep -a -x Xorg                 # which display sits on that VT
```

`idle-suspend.sh` now resolves DISPLAY from the active VT at startup and logs
it. If you switch VTs or start another X, restart the unit. `dock-monitor.sh`
does the same resolution on every hotplug event. The stale `:0` session was
killed on 2026-09-18 and dunst was restarted onto `:1`. Both user units still
carry `Environment=DISPLAY=:0`; the scripts override it, so that line is
harmless but misleading.

Remaining wake source: the wch.cn USB touchscreen (Cadwell 27c0:0859, on the
strip panel) drops off USB when its panel loses signal and re-enumerates ~2 s
later. The X input hotplug resets the idle counter and wakes DPMS, so panels
come back within seconds. Fixed 2026-09-18 with
`/etc/udev/rules.d/99-ignore-wch-touchscreen.rules`, which clears `ID_INPUT`
for USB 27c0:0859 so X and libinput never add it. Touch on the strip panel is
gone on purpose. Delete that rule and run `udevadm trigger` to get it back.

## Third finding: plain DPMS is the right answer (gamma hack was a mistake)

I briefly concluded DPMS could not be used and switched the script to a
gamma-black + backlight-off workaround. That was WRONG and is what the user
complained about: `xrandr --brightness 0` leaves the backlight lit, and waking
re-ran a layout so recovery was slow. Reverted 2026-09-19.

DPMS works fine once the two real confounds are gone:
1. The service was talking to the wrong X server (`:0` vs the live `:1`) — see
   the DISPLAY-from-active-VT fix above.
2. The strip's touchscreen re-enumerated on panel power-down and reset the idle
   counter — fixed by the udev ignore rule above.

Verified 2026-09-19 on `:1`: `xset dpms force off` put eDP-1, DP-1 and DP-2 all
to kernel DPMS `Off` (backlights off) and they STAYED off while the idle
counter climbed 154k -> 162k ms with no input; `xset dpms force on` woke all
three instantly. The strip does still drop and re-add its touchscreen on USB
when it powers down, but libinput ignores that device so the idle counter is
untouched.

Final design in `idle-suspend.sh`: `panels_off() { xset dpms force off; }`,
re-forced every 30 s poll while idle >= 300 s; real keyboard/mouse input wakes
everything via X's own DPMS. `IDLE_SUSPEND_SEC=0` keeps it panels-only so the
host stays reachable over the network.

Verification caveat: the idle counter cannot be tested while the user is typing
in a terminal on `:1` — that IS input and resets it. A real test is leaving the
desk for 5 minutes and reading `journalctl --user -u idle-suspend` for the
"DPMS off" line.

## How to check next time

```bash
systemctl --user status idle-suspend.service          # must be active, "panels off after 300s"
journalctl --user -u idle-suspend.service -n 20       # look for "forcing DPMS off" lines
DISPLAY=:0 ~/.local/bin/xidle                         # idle ms; large number + lit panels = this bug
DISPLAY=:0 xset q | rg "Monitor is"                   # "Off" while panels are lit = stale DPMS state
```

Anything that calls xrandr (dock-monitor.sh, dwm-hotswap.sh, lewis-layout.sh,
tri-layout.sh, mobile-layout.sh) can re-light the panels without X noticing.
Do not add a "monitor is on" guard back to the script.
