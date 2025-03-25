import * as esbuild from 'esbuild';
import { existsSync, mkdirSync } from 'fs';
import { dirname } from 'path';

// 确保输出目录存在
const outdir = './dist';
if (!existsSync(outdir)) {
  mkdirSync(outdir, { recursive: true });
}

// esbuild 配置
const buildOptions = {
  entryPoints: ['./src/index.ts'],
  outfile: './dist/index.js',
  bundle: true,
  sourcemap: true,
  minify: process.env.NODE_ENV === 'production',
  platform: 'browser',
  format: 'esm',
  target: ['esnext'],
};

// 运行构建
try {
  await esbuild.build(buildOptions);
  console.log('✅ JS 构建成功!');
} catch (error) {
  console.error('❌ 构建失败:', error);
  process.exit(1);
}
