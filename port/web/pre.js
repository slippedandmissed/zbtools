// Runs before the game (--pre-js): mounts C: on IndexedDB, so what the game
// saves (its roster, Zoombini.who) survives the page, and fills it with the
// installed files the data package carries (/c-default) that it lacks; and
// waits for a click, which also lets the browser start the sound.
//
// zoombinis-config.js sets Module.zbArguments (the drives); `uv run port
// package` writes it and zoombinis-data.js (the game's files, as /d and
// /c-default) from the user's copy of the game.

Module.arguments = (Module.zbArguments || []).slice();
// ?screenshot: the game's screen, as /screenshot.bmp (for testing).
if (typeof location !== 'undefined' && /[?&]screenshot\b/.test(location.search))
  Module.arguments.push('--screenshot', '/screenshot.bmp');
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
  var start = document.getElementById('start');
  if (start) {
    Module.setStatus('Ready.');
    start.disabled = false;
    start.onclick = function () {
      document.getElementById('overlay').style.display = 'none';
      Module.canvas.focus();
      removeRunDependency('zb-start');
    };
  } else
    removeRunDependency('zb-start');
});

// Once the data package has loaded too (it's a run dependency of its own),
// just before the game starts: fill C: with what it lacks.
Module.onRuntimeInitialized = function () {
  copyMissing('/c-default', '/c');
  Module.zbPersist();
};

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
