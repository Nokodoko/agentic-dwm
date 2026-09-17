# phyOS-dwm Visual Style Port — Implementation Plan

Goal: bring phyOS-dwm's **look** (tall segmented/floating bar, big gaps,
per-block colored-icon status, thicker focus-colored borders, optional client
icons) into `/home/n0ko/bling/dwm` while **fully preserving the existing
workflow** (pertag, tri-monitor pinning, six scratchpads, status2d/dwmblocks
pipeline, labeled floating borders, keybinds, layouts).

phyOS externalizes config into `.so`/libconfig and uses a 3-window split bar.
We are **not** adopting that architecture. We cherry-pick the visual cues and
graft them onto the user's single-`barwin` dwm 6.6 tree.

Legend — Risk: 🟢 trivial config / 🟡 contained C edit / 🔴 invasive C surgery.
Delivery: **[PATCH]** = in `phyos-style.patch` (validated) | **[MANUAL]** =
guided C edit below.

---

## Phase 0 — Safe appearance layer (SHIPPED in phyos-style.patch) 🟢

These are pure value swaps in `config.h`. No geometry logic changes. Validated
with `git apply --check` (PASS).

| Cue | phyOS source | User integration point | Change |
|-----|--------------|-------------------------|--------|
| Thick borders | `pdwmc/appearance:1` `borderpx=3` (applied `pdwm.c:950,1635-1636`) | `config.h:6` `borderpx` | `2 → 3` |
| Bigger gaps | `appearance:2-6` gappih=20/gappiv=10/gappoh=15/gappov=8 (consumed `vanitygaps.c:66-69`) | `config.h:7` `gappx` (single-gap `tile()` `dwm.c:2137-2163`) | `2 → 12` |
| Snap | `appearance` `snap=25` | `config.h:8` `snap` | `32 → 25` |
| Focus border color | `colors.h` blue `#96CDFB` → `SchemeSel[ColBorder]` (`pdwm.c:1273`) | `config.h:17` `col_lightblue` (feeds `SchemeSel` border `config.h:24`) | `#ADD8E6 → #96CDFB` |

Why these are safe: `borderpx`, `gappx`, `snap` are read by existing code paths
unchanged; `col_lightblue` only feeds the `SchemeSel` border slot the user
already defined. The single-gap `tile()` simply uses the larger `gappx` value —
no new math required.

**Note on config.def.h:** the build compiles `config.h` (newer, hand-edited:
has `SchemeOlr/AI/Steam`, 15 tags, tri-monitor rules). `config.def.h` is the
stale stock template (different scheme enum, no custom schemes). The patch
targets **config.h only** — do not regenerate config.h from config.def.h or you
will lose the workflow.

---

## Phase 1 — Tall bar 🟡 [MANUAL]

phyOS fixes `bh=user_bh=40` independent of font (`pdwm.c:2319`:
`bh = user_bh ? user_bh : drw->fonts->h+2`). The user's tree hardcodes
`bh = drw->fonts->h + 2` at **dwm.c:1947**.

This is the highest-value single visual change and is low-risk because every
vertical-centering call in this dwm already uses `bh` as the height argument
(`drw_text(..., bh, ...)` in `drawbar` `dwm.c:927-948`, `drawstatusbar(m, bh,
stext)` `dwm.c:917`). Raising `bh` reflows automatically.

**Step 1a — add a knob in config.h** (top of appearance block, ~line 11):
```c
static const int user_bh = 30;   /* bar height override; 0 = font height + 2 (phyOS uses 40) */
```

**Step 1b — consume it in setup() at dwm.c:1947:**
```c
/* before */
bh = drw->fonts->h + 2;
/* after */
bh = user_bh ? user_bh : drw->fonts->h + 2;
```

Risk note 🟡: text inside the bar is vertically centered by `drw_text`'s own
`(h - fonts->h)/2` math, so a taller `bh` centers text correctly. The tag
occupancy box (`boxs/boxw`, `dwm.c:908-909`) is derived from font height, not
`bh`, so it stays small and correct. Verify `dmenu`/launcher height separately —
those live in external scripts, not dwm.

---

## Phase 2 — Bar horizontal padding (faux "floating" inset) 🟡 [MANUAL]

phyOS's bar is visibly inset from the screen edge (`sb_padding_x=12`, barwin at
`m->wx+sb_padding_x`, width `m->ww-2*sb_padding_x`, `pdwm.c:2711-2723`). The
user's bar is edge-to-edge (`XMoveResizeWindow(... m->wx, m->by, m->ww, bh)` and
`drw_map(drw, m->barwin, 0, 0, m->ww, bh)`).

A **true** 3-window floating bar is Phase 5 (not recommended). A cheap visual
approximation: inset the single barwin by a horizontal pad. This is 🟡 because
it touches barwin geometry AND the status hit-test math (`statusw`, `dwm.c:917`,
`buttonpress` `dwm.c:494-509`) must stay consistent.

**Step 2a — config.h knob:**
```c
static const int sb_padding_x = 0;  /* bar horizontal inset; set 12 for phyOS float look */
```
(Default 0 keeps current behavior; opt in by raising it.)

**Step 2b — barwin creation/resize** wherever `m->ww`/`m->wx` set the barwin
(updatebars `dwm.c:2279-2286`, plus the repositions near `dwm.c:649,2170`):
use `m->wx + sb_padding_x` for x and `m->ww - 2*sb_padding_x` for width.

**Step 2c — drawbar** `dwm.c`: the final `drw_map(drw, m->barwin, 0, 0, m->ww,
bh)` must blit width `m->ww - 2*sb_padding_x`, and `tw = statusw = (m->ww -
2*sb_padding_x) - drawstatusbar(...)` so click hit-testing stays aligned.

Risk note 🟡→🔴: every place that compares pointer-x against bar regions must
subtract `sb_padding_x`. Miss one and clicks land on the wrong block. Audit all
`m->ww` references in `drawbar`, `buttonpress`, and `wintomon` before shipping.
**Recommended:** ship Phase 0+1 first, live with edge-to-edge bar, evaluate
whether the inset is worth the click-math audit.

---

## Phase 3 — Per-block colored-icon status 🟡 [MANUAL, mostly outside dwm.c]

The user ALREADY has the hard part: `drawstatusbar()` (`dwm.c:790-902`) parses
status2d `^c/^b/^d/^r/^f` escapes and allocates a dedicated status scheme slot
(`scheme[LENGTH(colors)]`, `dwm.c:1970-1971`). phyOS achieves its colored-block
look through `'|'`-delimited blocks where text ending in `%` flips to
`SchemeCritical`(<=30) / `SchemeOptimal` (`pdwm.c:1105-1116`) and PNG icons per
block (`drw_pic`, Imlib2, `drw.c:522-560`).

**Recommended approach — do it in the BLOCKS, not dwm.c.** The user's status is
fed by dwmblocks at `/home/n0ko/bling/dwmblocks/blocks.h` (16 blocks). status2d
`^c<#hex>` already lets each block emit its own color. To match phyOS:

1. Give each block script a nerd-font glyph prefix + its own `^c<#hex>` color,
   e.g. battery script emits `^c#F28FAD^ 12%^d` when low, `^c#ABE9B3^ 80%^d`
   otherwise (mirrors phyOS Critical `#F28FAD` / Optimal `#ABE9B3`).
2. Use `^r<x,y,w,h>` (already parsed `dwm.c:872-881`) to draw per-block
   background pills/separators if desired.

This needs **zero dwm.c change** — it is a blocks.h + script edit. 🟢 for the
pipeline, 🟡 only if you also want true PNG (raster) icons.

**Optional — raster PNG icons (drw_pic):** phyOS loads PNGs via Imlib2
(`drw.c:522-560`, `drw_pic` `drw.c:201-210`). Porting this means adding Imlib2
+ XRender Picture code to the user's `drw.c`/`drw.h` and an `^I<idx>` style
escape to `drawstatusbar`. 🔴 — significant surface, new lib dependency
(`-lImlib2`), and the user's nerd-font glyph approach already gives icon-like
status for free. **Defer unless raster logos are specifically wanted.**

---

## Phase 4 — Per-client title icons (NetWMIcon) 🔴 [MANUAL]

phyOS reads `_NET_WM_ICON`, premultiplies alpha (`prealpha` `pdwm.c:1820-1826`),
scales via XRender bilinear with Imlib2 downscale fallback
(`drw_picture_create_resized` `drw.c:108-199`), stores `c->icon/icw/ich`
(`geticonprop` `pdwm.c:1828-1905`), and blits left of the title (`drawbar
1184-1195`).

Integration into the user's tree:
- Add `c->icon`, `c->icw`, `c->ich` to the `Client` struct (`dwm.c` Client def).
- Add `geticonprop`/`updateicon`/`freeicon`, call `updateicon` from
  `manage()` and a `PropertyNotify` on `_NET_WM_ICON`.
- Add `drw_pic` + `drw_picture_create_resized` to `drw.c` (XRender; Imlib2 only
  if you want the >2x downscale fallback).
- In `drawbar` title region (`dwm.c:941-944`), draw `c->icon` before
  `m->sel->name` and shift text right by `icw + pad`.

Risk 🔴: new XRender/Imlib2 code in drw.c, struct change, extra link flags, and
it touches `drawbar` title layout. **This is a standalone mini-project — do it
last, in isolation, after Phases 0-1 are confirmed stable.** It does NOT
interact with pertag/scratchpads/borders, so it is risk-isolated but large.

---

## Phase 5 — True 3-window split/floating bar 🔴 NOT RECOMMENDED

phyOS builds three override-redirect windows (`allbarwin[0]` centered title,
`allbarwin[1]` status+systray, `m->barwin` logo/tags/ltsymbol) created in
`updatebars` (`pdwm.c:2688-2752`) and repositioned every `drawbar`. This is the
single largest divergence from the user's tree, and it directly collides with:
- `statusw` click hit-testing (`dwm.c:494-509,917`),
- per-monitor `m->barwin` geometry assumed single-window everywhere,
- `togglebar`, `updategeom`, `configurenotify` bar repositioning.

Porting it means rewriting bar setup + draw + click routing wholesale — exactly
the "config architecture replacement" the constraints forbid. **Skip.** Phases
1+2 (tall bar + optional inset) deliver ~80% of the perceived phyOS bar look at
a fraction of the risk.

---

## Animations — NOT a dwm change ⚠️

phyOS-dwm's README line 3 states verbatim: *"For full animation support, please
install `phyOS-picom` fork. Any other picom fork won't work."* The recon
confirmed **there is NO animation/tween/frame-loop code in the phyOS dwm C
source** — `drawbar`/`drw_map` are event-driven one-shot draws. All window
fades/slides come from the external **phyOS-picom compositor fork**.

Therefore **no edit to dwm.c, config.h, or this patch can produce animations.**
To get the phyOS motion look, install an animation-capable picom:

- `phyOS-picom` (exact match to phyOS), **or**
- `FT-Labs/picom` (actively maintained animations fork), **or**
- `pijulius/picom`.

Sample `~/.config/picom.conf` snippet (FT-Labs/picom syntax):
```ini
# --- animations ---
animations = true;
animation-stiffness = 200;
animation-window-mass = 0.5;
animation-dampening = 25;
animation-clamping = true;
animation-for-open-window    = "slide-up";
animation-for-unmap-window   = "slide-down";
animation-for-transient-window = "slide-up";

# --- fade (works on stock picom too) ---
fading = true;
fade-in-step = 0.06;
fade-out-step = 0.06;
fade-delta = 8;

# --- corners + shadow to match phyOS softness ---
corner-radius = 10;
round-borders = 1;
shadow = true;
shadow-radius = 14;
shadow-opacity = 0.55;
```
Launch from the user's autostart (e.g. `picom --config ~/.config/picom.conf &`).
This is the **only** path to animations; it lives entirely outside this repo.

---

## What we are deliberately NOT changing (workflow preserved) ✅

These are the user's identity and are explicitly left untouched by both the
patch and all recommended phases:

- **pertag** struct and per-tag nmaster/mfact/layout/showbar state
  (`dwm.c:300-301`, view/tag/setlayout integration). Untouched.
- **Tri-monitor tag pinning**: `defaulttags[]`, `tagmonmap[]`, `viewmon`
  (`config.h:126-137`) and the `TAGKEYS` macro using `viewmon`. Untouched.
- **Six class-routed scratchpads** + `scratchvisible` bitmask `ISVISIBLE`
  (`dwm.c:53`, `togglescratch`). Untouched — Alt+s/b/o/a/r and Alt+Shift+s
  binds preserved.
- **status2d/dwmblocks pipeline**: `drawstatusbar` escape parser, `statuscmd`
  click routing, `sigstatusbar`, `STATUSBAR "dwmblocks"`. Preserved and reused
  (Phase 3 builds on it, does not replace it).
- **Labeled floating borders** `drawbordertitle` + per-rule
  `borderscheme/bordertitle/floatw/floath` and `SchemeOlr/AI/Steam`
  (`dwm.c:964-1004`, `config.h:60-122`). Untouched. (The blue `#96CDFB` change
  only affects `SchemeSel` border via `col_lightblue`; the teal/green/purple
  overlay border schemes keep their distinct colors.)
- **All keybinds, layouts (`tile`/`monocle`/floating), moveresize nudging,
  custom spawn scripts.** Untouched.
- **No swallow, no systray** added (the user's tree has neither; phyOS's are not
  ported — they are workflow/feature, not "look").
- **config architecture**: we keep the single `config.h` + `dwm.c`
  compile-time model. We do NOT adopt phyOS's `.so`/libconfig externalized
  config. No `physettings`, no `dwm-conf.c`.

---

## Recommended execution order

1. **Apply `phyos-style.patch`** (Phase 0). Rebuild, hotswap, confirm look.
2. **Phase 1** (tall bar, 2-line C edit) — biggest visual win, lowest C risk.
3. **Phase 3 via blocks.h** (colored glyph status) — zero dwm.c risk.
4. **Phase 2** (bar inset) only if the floating look is wanted, after the
   click-math audit.
5. **Phase 4** (client icons) as an isolated mini-project, last.
6. **Phase 5** — skip.
7. **Animations** — install picom fork + config; independent of all the above.
