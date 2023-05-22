import { Socket } from 'net'
import { RequestMethod } from './enums';
import { generateRandomMatrix } from './helpers';
import { BYTE_SIZE, INT_SIZE, MATRIX_SIZE, PRINT_MATRIX_THRESHOLD } from './constants';

export const sendData = (socket: Socket) => {
  const matrix = generateRandomMatrix(MATRIX_SIZE);

  if (matrix.length <= PRINT_MATRIX_THRESHOLD) {
    console.log(matrix);
  }

  const buf = Buffer.alloc(
    BYTE_SIZE + // method
    INT_SIZE + // matrixSize
    matrix.length * matrix.length * INT_SIZE // matrix
  );
  let offset = 0;

  buf.writeUint8(RequestMethod.SET_DATA, offset);
  offset += BYTE_SIZE;

  buf.writeUint32LE(matrix.length, offset);
  offset += INT_SIZE;

  for (const row of matrix) {
    for (const el of row) {
      buf.writeInt32LE(el, offset);
      offset += INT_SIZE;
    }
  }

  socket.write(buf);
}

export const sendRequest = (socket: Socket, method: RequestMethod) => {
  socket.write(Buffer.from([method]));
}
