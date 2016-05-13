chrome.app.runtime.onLaunched.addListener(function() {
  chrome.app.window.create('vectrui.html', {
    'outerBounds': {
      'minWidth': 1024,
      'minHeight': 640,
      'maxWidth': 1024,
      'maxHeight': 640
    }
  });
});
