const CREATE_CACHES= false;

async function createCaches() {
  let cache1 = await caches.open('cache1')
  await cache1.add('/figures/cat.jpg');
  await cache1.add('/figures/cow.jpg')
  let cache2 = await caches.open('cache2')
  await cache2.addAll([
    '/figures/dog.jpg',
    '/figures/cat.jpg'
  ]);


  const response = await fetch('favicon.ico')
  await cache1.put('favicon.ico', response);
  console.log("Finish creating caches");
}


self.addEventListener('install', (event) => {
  console.log("installed event called", event);
  event.waitUntil(createCaches())
});

self.addEventListener('activate', (event) => {
  console.log("Activate event called", event)
});


self.addEventListener('fetch',(event) => {
  console.log("SW fetching", event.request.url);
  const path = '/' + event.request.url.split('/').slice(3).join('/');
  event.respondWith(caches.match(event.request)
    .then((cachedResponse) => {
      if (cachedResponse !== undefined) {
        return cachedResponse;
      } else {
        console.log("Cache miss for: ", event.request.url);
        return fetch(event.request)
      }
    })
  );
})
