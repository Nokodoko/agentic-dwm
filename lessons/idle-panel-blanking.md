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

## Third finding: DPMS itself cannot be used with the strip monitor

With the touchscreen ignored, `xset dpms force off` still lasted ~3 s. The
DP-2 strip (EDID vendor CRX, USB-C portable) drops its DisplayPort link when
it loses signal and reconnects a few seconds later. modesetting re-reads its
EDID on that hotplug and re-lights every CRTC, and the X idle counter resets.
Hardware behaviour, not fixable in software.

Final design in `idle-suspend.sh` (`IDLE_BLANK_MODE=gamma`, the default):
`xrandr --output <each> --brightness 0` on every monitor plus
`bl_power=4` on `/sys/class/backlight/amdgpu_bl1` via `sudo -n`. Panels stay
powered but black, nothing re-enumerates. Input restores brightness 1 and
`bl_power=0`. `IDLE_BLANK_MODE=dpms` in the unit brings back real DPMS if the
strip monitor is ever gone.

Verification caveat: the idle counter cannot be tested while the user is
typing. A real test is leaving the desk for 6 minutes and reading
`journalctl --user -u idle-suspend` for "blanking panels" / "restoring panels".

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
