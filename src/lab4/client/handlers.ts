import { Socket } from 'node:net';
import { RequestMethod, Status } from './enums';
import { sendData, sendRequest } from './requests';
import { GET_RESULT_INTERVAL } from './constants';

type Handler = (socket: Socket, data?: Buffer) => void;

let timer: NodeJS.Timer;

const handleConnectionEstablished: Handler = socket => {
  console.log('Connection established');

  sendData(socket);
}

const handleDataReceived: Handler = socket => {
  console.log('Server received data');

  sendRequest(socket, RequestMethod.START_PROCESSING);

  timer = setInterval(() => sendRequest(socket, RequestMethod.GET_RESULT), GET_RESULT_INTERVAL)
}

const handleResultInProgress: Handler = () => {
  console.log('Result in progress');
}

const handleServerError: Handler = (_, data) => {
  clearInterval(timer);
  console.log('Server got an error' + (data ? data.toString().slice(1) : ''));
}

const handleServerSuccess: Handler = (socket, data) => {
  clearInterval(timer);

  console.log('Got result from server: ');

  const duration = data?.readInt32LE(1);

  if (duration) {
    console.log(`Server took ${duration} ms to handle the data`);
  }

  sendRequest(socket, RequestMethod.END);
}

export const handlersMap: Record<Status, Handler> = {
  [Status.CONNECTION_ESTABLISHED]: handleConnectionEstablished,
  [Status.DATA_RECEIVED]: handleDataReceived,
  [Status.RESULT_IN_PROGRESS]: handleResultInProgress,
  [Status.ERROR]: handleServerError,
  [Status.SUCCESS]: handleServerSuccess,
};
