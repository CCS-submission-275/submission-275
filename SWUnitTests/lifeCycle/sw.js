const CREATE_CACHES= false;


self.addEventListener('install', (event) => {
  console.log("installed event called", event);
});

self.addEventListener('activate', (event) => {
  console.log("Activate event called", event)
});
