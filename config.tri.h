/* config.tri.h -- "singletag" variant: docked 3-monitor layout on the
 * suckless single_tagset port (branch 'singletag', ~/bling/dwm-singletag).
 *
 * Built by `dwm-hotswap.sh singletag`, selected automatically by
 * dock-monitor.sh / .xinitrc when two non-4K externals are connected, and
 * by ~/.screenlayout/3monitors.sh (-> ~/scripts/tri-layout.sh).
 *
 * One shared tagset, THREE visible tags, and each monitor views exactly one
 * of them, so "a monitor is a tag": Super+1/2/3 focuses that monitor,
 * Super+Shift+1/2/3 sends the window there. Windows open on the focused
 * monitor unless a rule says otherwise. The six trailing tags are hidden
 * scratchpad tags.
 *
 * Monitor numbering follows Xinerama order (xrandr --listmonitors); the
 * tag numbering follows the physical layout left-to-right, top-to-bottom:
 *   mon 1 / tag 1  DP-1   1920x1200 @ 0x400      left
 *   mon 0 / tag 2  eDP-1  2560x1600 @ 1920x0     primary, top right
 *   mon 2 / tag 3  DP-2   2560x720  @ 1920x1600  strip, below eDP-1
 */
/* See LICENSE file for copyright and license details. */

#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 3;        /* border pixel of windows (phyOS: 3) */
static const unsigned int gappx     = 12;       /* gap pixel between windows (phyOS inner ~10-20) */
static const unsigned int snap      = 25;       /* snap pixel (phyOS: 25) */
static const int user_bh            = 34;       /* bar height; 0 = auto (font height + 2). phyOS uses 40. Set 0 to revert. */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
/* size 9: kept in sync with config.single.h -- see the ColPx note there.
 * A config switch must not silently change the bar's column width. */
static const char *fonts[] = {"VictorMono Nerd Font Mono:style=Italic:size=9"};
static const char dmenufont[] = "VictorMono Nerd Font Mono:style=Italic:size=11";
static const char col_gray1[] = "#000000";
static const char col_gray2[] = "#333333";
static const char col_gray3[] = "#5F5F00";
static const char col_gray4[] = "#00FF00";
static const char col_cyan[] = "#000000";
static const char col_lightblue[] = "#96CDFB";
static const char col_teal[] = "#0088AA";
static const char col_green[] = "#00AA44";
static const char col_purple[] = "#9932CC";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_lightblue  },
	[SchemeOlr]  = { col_gray3, col_gray1, col_teal  },
	[SchemeAI]   = { col_gray3, col_gray1, col_green },
	[SchemeSteam] = { col_gray3, col_gray1, col_purple },
};

/* tagging — one tag per monitor: left DP-1 (web icon), laptop eDP-1 (term icon),
 * HDMI-1 (desktop icon, tag 3), strip DP-2 (music icon, tag 4).
 * createmon() refuses a monitor whose tag bit lands in SCRATCHTAGS, so there
 * must be at least as many real tags here as monitors (4 docked). */
static const char *tags[] = { "\xef\x82\xac", "\xef\x92\x89", "\xef\x84\x9b", "\xef\x80\x81", "SP", "SP2", "OLR", "AI", "STM", "SSH" };
#define SCRATCHPAD_TAG (1 << (LENGTH(tags) - 6))
#define BTOP_SCRATCHPAD_TAG (1 << (LENGTH(tags) - 5))
#define OLR_SCRATCHPAD_TAG (1 << (LENGTH(tags) - 4))
#define AI_SCRATCHPAD_TAG (1 << (LENGTH(tags) - 3))
#define STEAM_SCRATCHPAD_TAG (1 << (LENGTH(tags) - 2))
#define SSH_SCRATCHPAD_TAG (1 << (LENGTH(tags) - 1))

static const Rule rules[] = {
	/* xprop(1):
	 *   WM_CLASS(STRING) = instance, class
	 *   WM_NAME(STRING) = title
	 *
	 * singletag rules: a rule's tags mask picks the monitor (applyrules
	 * finds whoever views that tag; the monitor column is only the fallback
	 * when no monitor views it); tags 0 + monitor -1 means "open where
	 * focus is".
	 *   tag 1 (1<<0)  DP-1   (mon 1)  browsers, chat
	 *   tag 2 (1<<1)  eDP-1  (mon 0)  terminals, agents, games
	 *   tag 3 (1<<2)  HDMI-1 (mon 3)  right-hand 1080p
	 *   tag 4 (1<<3)  DP-2   (mon 2)  media strip
	 *   Scratchpads: floating, monitor -1 (follow focus)
	 */
	/* class              instance  title           tags mask              isfloating  monitor  iscentered  bw  borderscheme  bordertitle  floatw  floath */

	/* --- Floating utilities (follow focus) --- */
	{"Gimp",              NULL,     NULL,           0,                     1,          -1,      0,          -1, -1,           NULL,        1200,   900},

	/* --- Scratchpads: floating, follow focus (mon -1) --- */
	{"term-scratchpad",   NULL,     NULL,           SCRATCHPAD_TAG,        1,          -1,      0,          -1, -1,           NULL,        1200,   900},
	{"btop-scratchpad",   NULL,     NULL,           BTOP_SCRATCHPAD_TAG,   1,          -1,      0,          -1, -1,           NULL,        1200,   900},
	{"olr-scratchpad",    NULL,     NULL,           OLR_SCRATCHPAD_TAG,    1,          -1,      1,           1, SchemeOlr,    "olr",       1200,   900},
	{"ai-scratchpad",     NULL,     NULL,           AI_SCRATCHPAD_TAG,     1,          -1,      0,           1, SchemeAI,     "AI",        1200,   900},
	{"stm-scratchpad",    NULL,     NULL,           STEAM_SCRATCHPAD_TAG,  1,          -1,      1,           1, SchemeSteam,  "Steam",     1200,   900},
	{"ssh-scratchpad",    NULL,     NULL,           SSH_SCRATCHPAD_TAG,    1,          -1,      0,          -1, -1,           NULL,        1200,   900},

	/* --- Floating overlays (follow focus) --- */
	{"kitty-lf",          NULL,     NULL,           0,                     1,          -1,      1,           1, SchemeOlr,    "lf",        2200,   550},
	{"kitty-lister",      NULL,     NULL,           0,                     1,          -1,      1,           1, SchemeOlr,    "lister",    1100,   650},
	{"kitty-tabtiler",    NULL,     NULL,           0,                     1,          -1,      1,           1, SchemeOlr,    "tiles",     1200,   900},
	{"gpu-select",        NULL,     NULL,           0,                     1,          -1,      1,           1, SchemeOlr,    "gpu",        800,   500},
	{"trustgraph-viewer", NULL,     NULL,           0,                     1,          -1,      0,          -1, -1,           NULL,        0,      0},

	/* --- Browsers that follow focus (mon -1, current tag): grafana/datadog + webui on eDP-1 too --- */
	{"Vivaldi-stable",        NULL,     NULL,       0,                 0,          -1,      0,          -1, -1,           NULL,        0,      0},
	{"Vivaldi-flatpak",       NULL,     NULL,       0,                 0,          -1,      0,          -1, -1,           NULL,        0,      0},

	/* --- tag 1 / DP-1 (left, mon 1): browsers + chat --- */
	{"firefox",               NULL,     NULL,       1 << 0,            0,           1,      0,          -1, -1,           NULL,        0,      0},
	{"chromium",              NULL,     NULL,       1 << 0,            0,           1,      0,          -1, -1,           NULL,        0,      0},
	{"qutebrowser",           NULL,     NULL,       1 << 0,            0,           1,      0,          -1, -1,           NULL,        0,      0},
	{"Google-chrome",         NULL,     NULL,       1 << 0,            0,           1,      0,          -1, -1,           NULL,        0,      0},
	{"teams-for-linux",       NULL,     NULL,       1 << 0,            0,           1,      0,          -1, -1,           NULL,        0,      0},
	{"Slack",                 NULL,     NULL,       1 << 0,            0,           1,      0,          -1, -1,           NULL,        0,      0},
	{"discord",               NULL,     NULL,       1 << 0,            0,           1,      0,          -1, -1,           NULL,        0,      0},
	{"ZapZap",                NULL,     NULL,       1 << 0,            0,           1,      0,          -1, -1,           NULL,        0,      0},
	{"Electron",              NULL,     NULL,       1 << 0,            0,           1,      0,          -1, -1,           NULL,        0,      0},

	/* --- tag 2 / eDP-1 (laptop, mon 0): terminals, agents, games --- */
	{"St",                    NULL,     NULL,       1 << 1,            0,           0,      1,          -1, -1,           NULL,        0,      0},
	{"kitty",                 NULL,     NULL,       1 << 1,            0,           0,      0,          -1, -1,           NULL,        0,      0},
	{"neovide",               NULL,     NULL,       1 << 1,            0,           0,      0,          -1, -1,           NULL,        0,      0},
	{"wireshark",             NULL,     NULL,       1 << 1,            0,           0,      -1,         -1, -1,           NULL,        0,      0},
	{"cmdr-dashboard",        NULL,     NULL,       1 << 1,            0,           0,      0,          -1, -1,           NULL,        0,      0},
	{"cmdr-terminal",         NULL,     NULL,       1 << 1,            0,           0,      0,          -1, -1,           NULL,        0,      0},
	{"overstory-terminal",    NULL,     NULL,       1 << 1,            0,           0,      0,          -1, -1,           NULL,        0,      0},
	{"cmdr-feed",             NULL,     NULL,       1 << 1,            0,           0,      0,          -1, -1,           NULL,        0,      0},
	{"cmdr-costs",            NULL,     NULL,       1 << 1,            0,           0,      0,          -1, -1,           NULL,        0,      0},
	{"cmdr-logs",             NULL,     NULL,       1 << 1,            0,           0,      0,          -1, -1,           NULL,        0,      0},
	{"cmdr-errors",           NULL,     NULL,       1 << 1,            0,           0,      0,          -1, -1,           NULL,        0,      0},
	{"steam",                 NULL,     NULL,       1 << 1,            0,           0,      0,          -1, -1,           NULL,        0,      0},

	/* --- tag 4 / DP-2 (strip, mon 2): media --- */
	{"mpv",                   NULL,     NULL,       1 << 3,            0,           2,      0,          -1, -1,           NULL,        0,      0},

	/*
	 * Trustgraph / localhost:3000 — use Vivaldi app mode.
	 */
	{NULL,                    NULL,     "trustgraph", 0,               1,          -1,      1,          -1, -1,           NULL,        1200,   900},
	{NULL,                    NULL,     "localhost",  0,               1,          -1,      1,          -1, -1,           NULL,        1200,   900},
};

/* default tags per monitor (index = monitor number). With single_tagset
 * these MUST be pairwise distinct: a tag can only be viewed on one monitor. */
static const unsigned int defaulttags[] = {
    1 << 1,   /* mon 0 (eDP-1, 2560x1600):   tag 2 */
    1 << 0,   /* mon 1 (DP-1, 1920x1200):    tag 1 */
    1 << 3,   /* mon 2 (DP-2, 2560x720):     tag 4 */
    1 << 2,   /* mon 3 (HDMI-1, 1920x1080):  tag 3 */
};

/* layout(s) */
const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "[M]",      monocle },
	{ "><>",      NULL },    /* no layout function means floating behavior */
};

/* IPC */
static const char *ipcsockpath = "/tmp/dwm.sock";

/* Programs spawnsafe() may launch over IPC, matched against argv[0] exactly.
 *
 * The agent on the other end of the socket feeds window titles to a language
 * model, and window titles are attacker-controlled -- a web page picks its own.
 * This list is the boundary that keeps a prompt-injected model from execing
 * anything it likes. Note there is no shell: arguments are passed through
 * literally, so adding "sh", "bash" or "zsh" here would defeat the whole guard.
 * Extend deliberately. */
static const char *spawnallow[] = {
	"/home/n0ko/scripts/wezterm-egl-fix.sh",
	"/home/n0ko/scripts/lister.sh",
	"/usr/local/bin/monty",
	"wezterm",
	"vivaldi-stable",
	"btop",
	"nvim",
};

static IPCCommand ipccommands[] = {
  IPCCOMMAND(  view,                1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  toggleview,          1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  tag,                 1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  toggletag,           1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  tagmon,              1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  focusmon,            1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  focusstack,          1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  zoom,                1,      {ARG_TYPE_NONE}   ),
  IPCCOMMAND(  incnmaster,          1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  killclient,          1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  togglefloating,      1,      {ARG_TYPE_NONE}   ),
  IPCCOMMAND(  setmfact,            1,      {ARG_TYPE_FLOAT}  ),
  IPCCOMMAND(  setlayoutsafe,       1,      {ARG_TYPE_PTR}    ),
  IPCCOMMAND(  quit,                1,      {ARG_TYPE_NONE}   )
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      viewmon,        {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* status bar process name for click actions */
#define STATUSBAR "dwmblocks"

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, single monitor */
static const char *dmenucmd[] = { "/home/n0ko/scripts/lister.sh", NULL };
static const char *mountcmd[] = { "/home/n0ko/scripts/lister.sh", "mount", NULL };
static const char *termcmd[]  = { "kitty", NULL };
static const char *lyxcmd[]  = { "kitty", "--", "lyx", NULL };
static const char *killcmd[]  = { "killer.py", NULL };
static const char *pass[]  = { "pass.py", NULL };
static const char *wificmd[]  = { "wifi.py", NULL };
/* Super+Ctrl+Right: notification link picker. See config.single.h for why
 * hb.sh was replaced and why the path is absolute. */
static const char *hb[]  = { "/home/n0ko/.local/bin/statusbar/notif-open", NULL };
static const char *lb[]  = { "lb.sh", NULL };
static const char *pavucontrol[]  = { "pavucontrol", NULL };
static const char *dhp[]  = { "dhp.zsh", NULL };
static const char *mouseOn[]  = { "touchpadOn.sh", NULL };
static const char *mouseOff[]  = { "touchpadOff.sh", NULL };
static const char *volumeUp[]  = { "/home/n0ko/scripts/volume.sh", "up", NULL };
static const char *volumeDown[]  = { "/home/n0ko/scripts/volume.sh", "down", NULL };
static const char *volumeMute[]  = { "/home/n0ko/scripts/volume.sh", "mute", NULL };
static const char *cal[]  = { "kitty", "--", "calcurse", NULL };
static const char *top[]  = { "kitty", "--", "btop", NULL };
static const char *yazi[]  = { "/home/n0ko/scripts/fm-launcher.sh", "yazi", NULL };
static const char *scratchpadcmd[] = {"kitty", "--class", "term-scratchpad", NULL};
static const char *btopscratchpadcmd[] = {"kitty", "--class", "btop-scratchpad", "--", "btop", NULL};
static const char *olrscratchpadcmd[] = {"kitty", "--class", "olr-scratchpad", "--", "/usr/local/bin/olr", NULL};
static const char *aiscratchpadcmd[] = {"kitty", "--class", "ai-scratchpad", "--", "/usr/local/bin/monty", NULL};
static const char *steamscratchpadcmd[] = {"kitty", "--class", "stm-scratchpad", "--", "/home/n0ko/scripts/steam_launcher.zsh", NULL};
static const char *sshscratchpadcmd[] = {"kitty", "--class", "ssh-scratchpad", "--", "ssh", "-t", "base", "zellij", "attach", "-c", "default", NULL};
static const char *scrot_precision[] = { "/bin/sh", "-c", "scrot -s -e 'xclip -selection clipboard -t image/png -i $f && notify-send \"Screenshot Precision\" \"Copied to clipboard\"'", NULL };
static const char *slockcmd[] = { "/home/n0ko/scripts/slock-dpms.sh", NULL };
static const char *tgsnapcmd[] = { "/usr/local/bin/tg-snap", NULL };
static const char *lewislayoutcmd[] = { "/home/n0ko/scripts/tri-layout.sh", NULL };
static const char *restartdwm[] = { "/home/n0ko/scripts/dwm-hotswap.sh", "pertag", NULL };
static const char *restartdwm_wt[] = { "/home/n0ko/scripts/dwm-hotswap.sh", "singletag", NULL };
static const char *restartdwm_base[] = { "/home/n0ko/scripts/dwm-hotswap.sh", "base", NULL };
static const char *brightnessUp[] = { "/home/n0ko/scripts/brightnessUp.sh", NULL };
static const char *brightnessDown[] = { "/home/n0ko/scripts/brightnessDown.sh", NULL };
static const char *brightnessMid[] = { "/home/n0ko/scripts/brightnessMid.sh", NULL };
static const char *keybrightnessUp[] = { "/home/n0ko/scripts/keybrightnessUp.sh", NULL };
static const char *keybrightnessDown[] = { "/home/n0ko/scripts/keybrightnessDown.sh", NULL };
static const char *xboxConnect[] = { "/home/n0ko/scripts/xbox.sh", NULL };
static const char *vivaldileadercmd[] = { "/home/n0ko/.local/bin/vivaldi-leader.sh", NULL };
static const char *dwmleadercmd[] = { "/home/n0ko/.local/bin/dwm-leader.sh", NULL };

/* moveresize direction vectors: {dx, dy, dw, dh} */
static const int moveresizedelta_up[]    = {  0, -25,   0,   0 };
static const int moveresizedelta_down[]  = {  0,  25,   0,   0 };
static const int moveresizedelta_left[]  = { -25,  0,   0,   0 };
static const int moveresizedelta_right[] = {  25,  0,   0,   0 };
static const int moveresizedelta_sh[]    = {  0,   0,   0, -25 }; /* shrink height */
static const int moveresizedelta_gh[]    = {  0,   0,   0,  25 }; /* grow height */
static const int moveresizedelta_sw[]    = {  0,   0, -25,   0 }; /* shrink width */
static const int moveresizedelta_gw[]    = {  0,   0,  25,   0 }; /* grow width */

static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ Mod1Mask,                     XK_s,      togglescratch,  {.v = scratchpadcmd } },
	{ Mod1Mask,                     XK_b,      togglescratch,  {.v = btopscratchpadcmd } },
	/* Alt+o unbound 2026-07-10 (olr unused; Alt+o reserved for another app): { Mod1Mask, XK_o, togglescratch, {.v = olrscratchpadcmd } }, */
	{ Mod1Mask,                     XK_a,      togglescratch,  {.v = aiscratchpadcmd } },
        { Mod1Mask,                     XK_r,      togglescratch,  {.v = steamscratchpadcmd } },
	{ Mod1Mask|ShiftMask,           XK_s,      togglescratch,  {.v = sshscratchpadcmd } },
	{ Mod1Mask|ControlMask,         XK_v,      spawn,          {.v = vivaldileadercmd } },
	{ Mod1Mask,                     XK_t,      spawn,          {.v = dwmleadercmd } },
	{ Mod1Mask,                     XK_c,      spawn,          {.v = xboxConnect } },
	{ MODKEY,                       XK_F5,     spawn,          {.v = tgsnapcmd } },
	{ MODKEY,                       XK_r,      spawn,          {.v = restartdwm } },
	{ MODKEY|ShiftMask,             XK_r,      spawn,          {.v = restartdwm_wt } },
	{ MODKEY|ControlMask,           XK_r,      spawn,          {.v = restartdwm_base } },
	{ MODKEY|ControlMask,           XK_u,      spawn,          {.v = lewislayoutcmd } },
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_m,      spawn,          {.v = mountcmd } },
	{ MODKEY,                       XK_x,      spawn,          {.v = lyxcmd } },
	{ MODKEY|ShiftMask,             XK_x,      spawn,          {.v = killcmd } },
	{ MODKEY|ShiftMask,             XK_c,      spawn,          {.v = cal } },
	{ MODKEY,                       XK_m,      spawn,          {.v = pavucontrol } },
	{ MODKEY,                       XK_o,      spawn,          {.v = mouseOff } },
	{ MODKEY|ShiftMask,             XK_o,      spawn,          {.v = mouseOn } },
	{ 0,                            XF86XK_AudioRaiseVolume, spawn, {.v = volumeUp } },
	{ 0,                            XF86XK_AudioLowerVolume, spawn, {.v = volumeDown } },
	{ 0,                            XF86XK_AudioMute,        spawn, {.v = volumeMute } },
	{ MODKEY|ShiftMask,             XK_p,      spawn,          {.v = pass} },
	{ MODKEY|ShiftMask,             XK_w,      spawn,          {.v = wificmd} },
	{ Mod1Mask|ShiftMask,           XK_t,      spawn,          {.v = top} },
  { Mod1Mask|ControlMask,         XK_Down,   spawn,          {.v = dhp } },
  { Mod4Mask|ControlMask,         XK_Right,  spawn,          {.v = hb } },
  { Mod4Mask|ControlMask,         XK_Left,   spawn,          {.v = lb } },
  { Mod1Mask,                     XK_1,      spawn,          {.v = scrot_precision } },
  { Mod4Mask|ShiftMask,           XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY|ShiftMask,             XK_l,      spawn,          {.v = yazi } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_u,      incnmaster,     {.i = 0 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ControlMask,           XK_l,      spawn,          {.v = slockcmd } },
	{ 0,                            XF86XK_MonBrightnessUp,   spawn, {.v = brightnessUp } },
	{ 0,                            XF86XK_MonBrightnessDown, spawn, {.v = brightnessDown } },
	{ MODKEY|ControlMask,           XK_m,      spawn,          {.v = brightnessMid } },
	{ Mod1Mask|ControlMask,         XK_u,      spawn,          {.v = brightnessUp } },
	{ Mod1Mask|ControlMask,         XK_i,      spawn,          {.v = brightnessDown } },
	{ Mod1Mask|ControlMask,         XK_o,      spawn,          {.v = keybrightnessUp } },
	{ Mod1Mask|ControlMask,         XK_p,      spawn,          {.v = keybrightnessDown } },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ Mod4Mask|ShiftMask,           XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_Up,     moveresize,     {.v = moveresizedelta_up } },
	{ MODKEY,                       XK_Down,   moveresize,     {.v = moveresizedelta_down } },
	{ MODKEY,                       XK_Left,   moveresize,     {.v = moveresizedelta_left } },
	{ MODKEY,                       XK_Right,  moveresize,     {.v = moveresizedelta_right } },
	{ MODKEY|ShiftMask,             XK_Up,     moveresize,     {.v = moveresizedelta_gh } },
	{ MODKEY|ShiftMask,             XK_Down,   moveresize,     {.v = moveresizedelta_sh } },
	{ MODKEY|ShiftMask,             XK_Left,   moveresize,     {.v = moveresizedelta_sw } },
	{ MODKEY|ShiftMask,             XK_Right,  moveresize,     {.v = moveresizedelta_gw } },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)  /* tag 1: DP-1 (left)  */
	TAGKEYS(                        XK_2,                      1)  /* tag 2: eDP-1 (laptop) */
	TAGKEYS(                        XK_3,                      2)  /* tag 3: HDMI-1 (right) */
	TAGKEYS(                        XK_4,                      3)  /* tag 4: DP-2 (strip) */
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

/* button definitions */
static const Button buttons[] = {
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button1,        sigstatusbar,   {.i = 1} },
	{ ClkStatusText,        0,              Button2,        sigstatusbar,   {.i = 2} },
	{ ClkStatusText,        0,              Button3,        sigstatusbar,   {.i = 3} },
	{ ClkStatusText,        ShiftMask,      Button1,        sigstatusbar,   {.i = 6} },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
