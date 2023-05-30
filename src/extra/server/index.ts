import { createServer } from 'node:net';
import { createFloatBuffer, readFloatBuffer } from './float-buffer';
import { round } from './round';

const server = createServer(socket => {
  console.log(`Client connected`);

  socket.on('data', data => {
    console.log(`Server got: ${
      [...readFloatBuffer(data)]
        .map(el => round(el, 2))
        .join(', ')
    }`);
    socket.write(createFloatBuffer([1.1, 2.2, -3.33]));
  });

  socket.on('end', () => {
    console.log(`Client disconnected`);
  });

  socket.on('error', err => {
    console.log(`Socket error: ${err}`);
  });
});


server.listen(8080, () => {
  const address = server.address();
  console.log(`Server listening on port ${typeof address === 'object' ? address?.port : address}`);
});
