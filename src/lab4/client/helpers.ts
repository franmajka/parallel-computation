export const randomInRange = (min: number, max: number) => {
  return min + Math.floor(Math.random() * (max - min));
}

export const generateRandomMatrix = (size: number) => {
  return Array.from({ length: size })
    .map(
      () => Array.from({ length: size }).map(() => randomInRange(-size, size))
    );
}
