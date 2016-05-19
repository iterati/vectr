chrome.app.runtime.onLaunched.addListener(function() {
  chrome.app.window.create('index.html', {
    'innerBounds': {
      'minWidth': 1280,
      'minHeight': 720,
      'maxWidth': 1280,
      'maxHeight': 720
    }
  });
});
