export function concatUint8Array(...arr: (Uint8Array | undefined | null)[]): Uint8Array | undefined {
  const filteredArr = arr.filter(Boolean) as Uint8Array[];
  if (filteredArr.length < 2) return filteredArr[0];

  const data = new Uint8Array(filteredArr.reduce((p, c) => p + c.byteLength, 0));
  let prevLen = 0;
  filteredArr.forEach((d) => {
    data.set(d, prevLen);
    prevLen += d.byteLength;
  });
  return data;
} 