const check = () => {
  if (!('serviceWorker' in navigator)) {
    throw new Error('No Service Worker support!')
  }
  if (!('PushManager' in window)) {
    throw new Error('No Push API Support!')
  }
}

function insertNode(text) {
  resultsDiv = document.getElementById("results");
  const textNode = document.createTextNode(text);
  resultsDiv.appendChild(textNode);
}

const registerServiceWorker = async () => {
  const sw = await navigator.serviceWorker.register('sw.js');
  sw.addEventListener('statechange', (event) => {
    console.log(event);
    insertNode("statechange");
  });
}

const requestNotificationPermission = async () => {
  const permission = await window.Notification.requestPermission();
  // value of permission can be 'granted', 'default', 'denied'
  // granted: user has accepted the request
  // default: user has dismissed the notification permission popup by clicking on x
  // denied: user has denied the request.
  if (permission !== 'granted') {
    throw new Error('Permission not granted for Notification')
  }
}

const main = async () => {
  await registerServiceWorker();
}
//main(); //we will not call main in the beginning.
