// Runs before the game (--pre-js): mounts C: on IndexedDB, so what the game
// saves (its roster, Zoombini.who) survives the page, and fills it with the
// installed files the data package carries (/c-default) that it lacks; and
// waits for a click, which also lets the browser start the sound.
//
// zoombinis-config.js sets Module.zbArguments (the drives and the
// SoundFont); `uv run port package` writes it and zoombinis-data.js (the
// loaders of the packages holding the game's files, as /d and /c-default,
// and the SoundFont).

Module.arguments = (Module.zbArguments || []).slice();
// ?screenshot: the game's screen, as /screenshot.bmp (for testing).
if (typeof location !== 'undefined' && /[?&]screenshot\b/.test(location.search))
  Module.arguments.push('--screenshot', '/screenshot.bmp');
// ?cmd=...: debug commands to run at start (repeatable; see the handbook's
// "Debug tools"; builds without them ignore it), and zbDebug("...") runs
// more from the console.
if (typeof location !== 'undefined')
  new URLSearchParams(location.search).getAll('cmd').forEach(function (cmd) {
    Module.arguments.push('--cmd', cmd);
  });
window.zbDebug = function (commands) {
  // Queued here, not passed to the program: the game collects it on its next
  // frame (a call into the program while it sleeps would not be safe).
  (Module.zbDebugQueue = Module.zbDebugQueue || []).push(String(commands));
  return 'queued';
};
Module.preRun = Module.preRun || [];
Module.preRun.push(function () {
  addRunDependency('zb-drive-c');
  addRunDependency('zb-start');
  FS.mkdir('/c');
  FS.mount(IDBFS, { autoPersist: false }, '/c');
  FS.syncfs(true, function (error) {
    if (error)
      console.warn('IndexedDB unavailable; nothing will be saved:', error);
    removeRunDependency('zb-drive-c');
  });
  // The page's Play button (shell.html); without one, start at once.
  window.zbStart = function () {
    window.zbStart = null;
    var overlay = document.getElementById('overlay');
    if (overlay)
      overlay.style.display = 'none';
    Module.canvas.focus();
    removeRunDependency('zb-start');
  };
  if (window.zbStartClicked || !document.getElementById('start'))
    window.zbStart();
});

// Once the data packages have loaded too (each is a run dependency of its
// own), just before the game starts: put back together the files packed in
// parts, and fill C: with what it lacks.
Module.onRuntimeInitialized = function () {
  ['/c-default', '/d', '/soundfont'].forEach(joinParts);
  copyMissing('/c-default', '/c');
  Module.zbPersist();
};

// `uv run port package` splits files too big for a web host into NAME.part0,
// NAME.part1, ...: joins them back into NAME.
function joinParts(directory) {
  var entries;
  try {
    entries = FS.readdir(directory);
  } catch (e) {
    return;
  }
  entries.forEach(function (name) {
    var path = directory + '/' + name;
    // (A part already joined, and removed, is still in the list.)
    if (name === '.' || name === '..' || !FS.analyzePath(path).exists)
      return;
    if (FS.isDir(FS.stat(path).mode)) {
      joinParts(path);
      return;
    }
    var match = name.match(/^(.*)\.part0$/);
    if (!match)
      return;
    var base = directory + '/' + match[1], parts = [], size = 0;
    for (var n = 0; FS.analyzePath(base + '.part' + n).exists; n++) {
      var part = FS.readFile(base + '.part' + n);
      parts.push(part);
      size += part.length;
      FS.unlink(base + '.part' + n);
    }
    var whole = new Uint8Array(size), at = 0;
    parts.forEach(function (part) {
      whole.set(part, at);
      at += part.length;
    });
    FS.writeFile(base, whole);
  });
}

function copyMissing(from, to) {
  var entries;
  try {
    entries = FS.readdir(from);
  } catch (e) {
    return;
  }
  entries.forEach(function (name) {
    if (name === '.' || name === '..')
      return;
    var source = from + '/' + name, target = to + '/' + name;
    var isDirectory = FS.isDir(FS.stat(source).mode);
    var exists = FS.analyzePath(target).exists;
    if (isDirectory) {
      if (!exists)
        FS.mkdir(target);
      copyMissing(source, target);
    } else if (!exists)
      FS.writeFile(target, FS.readFile(source));
  });
}

// hostFilesChanged: persist C:, at most once a second.
(function () {
  var pending = false;
  Module.zbPersist = function () {
    if (pending)
      return;
    pending = true;
    setTimeout(function () {
      pending = false;
      FS.syncfs(false, function (error) {
        if (error)
          console.warn('saving to IndexedDB failed:', error);
      });
    }, 1000);
  };
})();
