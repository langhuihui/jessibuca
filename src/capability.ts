export function webCodecs(){
  return typeof AudioData !== 'undefined'
}
export function offscreenCanvas(){
  return typeof OffscreenCanvas !== 'undefined'
}