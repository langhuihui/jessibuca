enum JBEvent {
  create,
  start,
  stop,
  destroy,
  load,
  loaded,
  firstRender,
  report
}

interface JBReport {
  audio_bitrate: number;
  video_bitrate: number;
  video_receive_fps: number;
  video_decode_fps: number;
  video_render_fps: number;
  video_dropped_frames: number;
  width: number;
  height: number;
  rtt: number;
  audio_block_cnt: number;
  audio_block_time: number;
  video_block_cnt: number;
  video_block_time: number;
}