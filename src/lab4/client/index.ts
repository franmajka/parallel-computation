import { createConnection } from 'node:net';
import { Status } from './enums';
import { handlersMap } from './handlers';

const socket = createConnection({ port: 8080 }, () => {
  console.log('Client connected to server!');
});

socket.on('data', (data) => {
  const status: Status = data[0];

  handlersMap[status](socket, data);
});

socket.on('end', () => {
  console.log('Client disconnected');
});
