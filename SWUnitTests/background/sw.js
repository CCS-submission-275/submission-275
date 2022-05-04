const CREATE_CACHES= false;


self.addEventListener('install', (event) => {
  console.log("installed event called", event);
});

self.addEventListener('activate', (event) => {
  console.log("Activate event called", event)
});

self.addEventListener('sync', (event) => {
  console.log("event", event);
});

self.addEventListener('periodicsync', (event) => {
  console.log("periodicSync", event) 
})
