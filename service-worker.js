/*
 Copyright 2015 Google Inc. All Rights Reserved.
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 http://www.apache.org/licenses/LICENSE-2.0
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

'use strict';

// Incrementing CACHE_VERSION will kick off the install event and force previously cached
// resources to be cached again.
const CACHE_VERSION = 1;
let CURRENT_CACHES = {
   offline: 'offline-v' + CACHE_VERSION,
};
const OFFLINE_URL = 'index.html';

function createCacheBustedRequest(url) {
   let request = new Request(url, { cache: 'reload' });

   if ('cache' in request) {
      return request;
   }

   let bustedUrl = new URL(url, self.location.href);
   bustedUrl.search += (bustedUrl.search ? '&' : '') + 'cachebust=' + Date.now();
   return new Request(bustedUrl);
}

self.addEventListener('install', (event) => {
   event.waitUntil(
      fetch(createCacheBustedRequest(OFFLINE_URL)).then(function (response) {
         return caches.open(CURRENT_CACHES.offline).then(function (cache) {
            return cache.put(OFFLINE_URL, response);
         });
      })
   );
});

self.addEventListener('activate', (event) => {
   let expectedCacheNames = Object.keys(CURRENT_CACHES).map(function (key) {
      return CURRENT_CACHES[key];
   });

   event.waitUntil(
      caches.keys().then((cacheNames) => {
         return Promise.all(
            cacheNames.map((cacheName) => {
               if (expectedCacheNames.indexOf(cacheName) === -1) {
                  console.log('Deleting out of date cache:', cacheName);
                  return caches.delete(cacheName);
               }
            })
         );
      })
   );
});

self.addEventListener('fetch', (event) => {
   if (
      event.request.mode === 'navigate' ||
      (event.request.method === 'GET' && event.request.headers.get('accept').includes('text/html'))
   ) {
      console.log('Handling fetch event for', event.request.url);
      event.respondWith(
         fetch(event.request).catch((error) => {
            console.log('Fetch failed; returning offline page instead.', error);
            return caches.match(OFFLINE_URL);
         })
      );
   }
});
