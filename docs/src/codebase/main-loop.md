# The main loop, events and input

## One pass

`WinMain` runs `mainLoopUpdate()` and `mainLoopEvents()` alternately until `quitRequested` is set.

```text
mainLoopUpdate()   (mainloop.cpp)
  if gameActive:
      if pendingScene != -1      → enterNextScene()          (net.cpp: the scene switch)
      every ~3600 ticks          → halve snoidIdleDelay (Zoombinis fidget more)
      if cursor is the busy one  → return (nothing else this pass)
  if handleNextEvent()           → event queue, else the Win32 queue (handleNextMessage)
        a key → postKeyEvent → gameKey      a click → postMouseEvent → focus system
  else                           → handleMouse(cursor position): hover tracking

mainLoopEvents()   (debug.cpp)
  handleWaitingMessage()         (pumps one Win32 message through mainWindowProc)
  if !loadingAnimation:  frameHook()          = gameFrame()
  debug breakpoints, starvation check

gameFrame()        (game.cpp)
  setPort(workPort); scenes[currentScene]->frame(); setPort(saved)
  draw memory statistics; step the animated cursor every 12 ticks
```

**Everything in a scene happens in its `frame` callback**, reached through `gameFrame`, or in a callback a view or input group invokes. There are no game threads.

## Time

The clock counts 60ths of a second (`clockInTicks = 1`) through `clockTime()`. Views have their own clock (`viewClock`, `resetViewClock`) so a pause or a dialog doesn't age them. Timed behaviour is polled: a scene compares `clockTime()` with a stored due time in its frame. The engine's multimedia timers exist ([timers](engine-os-audio.md)) but the game rarely uses them directly.

## Events

`events.cpp` holds a 32-entry ring buffer of Mac-style events (key and mouse button). The Windows procedure turns `WM_CHAR`/`WM_KEYDOWN` and button-down messages into events (`handleMessage`, `mainWindowProc`); `handleNextEvent` takes one and dispatches it. `waitForEventFor(timer, ticks, type, discard)` is the blocking wait used by animations and dialogs: it pumps messages until an event of the type arrives or the time passes.

## Keys

`gameKey` (`mainloop.cpp`) notes each key for the cheat tracker, then:

1. gives it to the **dialog** if one is open (`dialogFlags`),
2. else to the **scene's `key` callback**, which returns whether it handled it,
3. else treats it as one of the game's own keys.

| Key | Action |
| --- | --- |
| Ctrl-N / Ctrl-L / Ctrl-S | new game / load game / save game |
| Ctrl-Q | quit (asks first) |
| Ctrl-B / Ctrl-D | music / sound on-off |
| Ctrl-G | "less action" / "more action" (Zoombinis fidget less) |
| Ctrl-H | hide / show the drag cursor |
| Ctrl-J | sticky / non-sticky mouse (click-to-drag vs hold) |
| Ctrl-T | screen transitions on/off |
| Ctrl-U | auto-sticky on/off |
| Ctrl-V | the about box |
| `/` or `?` | help dialog |

Each toggle shows a name tag from `toggleTexts` (`town.cpp`). A saved player's settings are applied at startup (`applyPlayerSettings`).

## Mouse and input groups: `focus.cpp`

On-screen controls are **items** (`InputItem`: bounds, hot spot, key, flags) arranged in **groups** (`Group`: handlers, items), which are listed in a **group list** (`GroupList`: groups, a click callback). Each scene calls `setGroupLists(groups, count, flags)` when it opens. The focus system:

- tracks the pointer (`handleMouse`), highlighting the item under it through the group's handlers (`enter`, `leave`, `hitTest`…);
- tracks a press like the Mac's `TrackControl` and calls the list's click callback with the item number when the button is released over the item that was pressed;
- moves focus with Tab / Shift-Tab and each item's `key`;
- supports on/off and exclusive items.

A scene's button array is therefore not just a list of rectangles: the scenes' `…Buttons[3]` arrays are the input group's items (two buttons and the whole screen), and the scene's `…Clicked(which)` function is the group list's callback. When a puzzle seems to have no code for "the user clicked the Go button", look for `<scene>Clicked`.

**Graphic buttons** (`buttons.cpp`) draw an item from its group's image pair (normal and lit); most scenes instead draw their own buttons (`drawBridgeButton`, `drawPizzaButton`, …) from a button image bank and redraw them from a view's `update` callback.

## Dragging

Zoombinis are dragged with the mouse. A **drag cursor** is a view (`viewTail`) that follows the mouse (`trackDragCursor`, `drawDragCursor` in `view.cpp`); "sticky mouse" makes a click pick up and a second click drop. `viewAt(point, mask, backwards)` is the hit test over the view list.

## Scene switching

`pendingScene` is set by anything that wants to go somewhere (`sceneDue` is a scene's own "I'm done, go to X" flag, copied to `pendingScene` by its frame). `enterNextScene` (in `net.cpp`) then:

1. records which camp a finished puzzle group should lead to (`puzzleLeft`, per-group "left" bits in `gameState`);
2. decides whether to show the **journey map** in between (`viaMap`: not in practice mode, not when `skipJourneyMap` or `transitionsOn` is set, and only when leaving a puzzle, camp or the isle for another place), setting `journeyFrom`/`journeyTo` and sending the game to scene 2;
3. sets `journeyFrom`/`currentScene`, notes that a new puzzle *group* has started (`gameState+0x54`, on entering scenes 7, 10, 13 or 16, the first puzzle of each group), marks the roster changed, and calls the new scene's `open()`.

A scene **closes itself**: its frame (or click handler) calls its own `close…` function (freeing its images, sounds, views and groups) and then sets `pendingScene`, so by the time `enterNextScene` runs the old scene is gone. `shutDownGame` closes whatever is still open.

See [Modules and scenes](modules-and-scenes.md) for the scene table.
