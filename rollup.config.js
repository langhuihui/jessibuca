import babel from 'rollup-plugin-babel'; // 大家都理解，方便使用 javascript 的新特性,es新特性的解析
import commonjs from 'rollup-plugin-commonjs'; // 插件将CommonJS模块转换为 ES2015 提供给 Rollup 处理
import postcss from 'rollup-plugin-postcss'; // 提取样式文件（sass 或者 less 等）添加浏览器前缀 以及压缩
import autoprefixer from 'autoprefixer'; // 添加前缀
import cssnano from 'cssnano'; // 压缩css
import replace from 'rollup-plugin-replace'; // 变量替换，可以将动态设置的变量提取出来在配置文件中设置
import nodeResolve from 'rollup-plugin-node-resolve';
import json from '@rollup/plugin-json'; // 解析json格式
import copy from 'rollup-plugin-copy';  // 直接复制文件和文件夹。
import {terser} from 'rollup-plugin-terser'; // 压缩
import base64 from 'postcss-base64'; // 图片变成base64
import packageJson from './package.json';


const isProd = process.env.NODE_ENV === 'production';

const baseConfig = {
    output: {
        sourcemap: !isProd,
    },
    plugins: [
        nodeResolve(),
        commonjs(),
        babel({
            runtimeHelpers: true,
            exclude: 'node_modules/**',
            presets: [
                [
                    '@babel/preset-env',
                    {
                        modules: false,
                    },
                ],
            ],
            plugins: ['@babel/plugin-external-helpers', '@babel/plugin-transform-runtime'],
        }),
        replace({
            exclude: 'node_modules/**',
            __ENV__: JSON.stringify(process.env.NODE_ENV || 'development'),
        }),
        isProd && terser({
            mangle: true,
            output: {
                comments: () => false,
            },
        }),
        isProd && {
            name: 'removeHtmlSpace',
            transform(code) {
                return {
                    code: code.replace(/\\n*\s*</g, '<').replace(/>\\n*\s*/g, '>'),
                };
            },
        },
        copy({
            targets: [
                {
                    src: "src/decoder/decoder.wasm",
                    dest: isProd ? 'dist' : 'demo/public'
                },
                {
                    src: 'd.ts/jessibuca.d.ts',
                    dest: isProd ? 'dist' : 'demo/public'
                }
            ],
            verbose: true, // Output copied items to console.
            copyOnce: true //  Copy items once. Useful in watch mode.
        })
    ],
};

export default [
    {
        input: 'src/jessibuca.js',
        output: [
            {
                name: 'jessibuca',
                file: isProd ? packageJson.main : 'demo/public/jessibuca.js',
                format: 'umd',
                ...baseConfig.output
            },
            {
                file: isProd ? packageJson.module : 'demo/public/jessibuca.js',
                format: 'esm',
                ...baseConfig.output
            }
        ],
        plugins: [
            postcss({
                plugins: [
                    base64({
                        extensions: ['.png'],
                        root: './src/control/',
                    }),
                    autoprefixer(),
                    cssnano({
                        preset: 'default',
                    }),
                ],
                sourceMap: !isProd,
                extract: false,
            }),
            json()
        ],
    },
    {
        input: 'src/worker.js',
        output: {
            name: 'decoder',
            file: isProd ? 'dist/decoder.js' : 'demo/public/decoder.js',
            ...baseConfig.output
        },
        plugins: [],
    }
].map(config => {
    return {
        input: config.input,
        output: config.output,
        plugins: [...baseConfig.plugins, ...config.plugins],
    };
});
