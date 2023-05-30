const FLOAT_SIZE = 4;

export const createFloatBuffer = (arr: number[] | Float32Array) => {
  const buf = Buffer.alloc(arr.length * FLOAT_SIZE);

  let offset = 0;
  for (const el of arr) {
    buf.writeFloatBE(el, offset);
    offset += FLOAT_SIZE;
  }

  return buf;
}

export function* readFloatBuffer(buf: Buffer) {
  for (let offset = 0; offset < buf.byteLength; offset += FLOAT_SIZE) {
    yield buf.readFloatBE(offset);
  }
}
