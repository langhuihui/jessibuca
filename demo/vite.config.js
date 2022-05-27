/**
 * @type {import('vite').UserConfig}
 */
import fs from 'fs';

const config = {
    server: {
        host: '0.0.0.0',
        port: 3001,
        https: {
            cert: fs.readFileSync('/Users/wancheng/Documents/GitHub/jessibuca-webTransport/demo/monibuca.com.pem'),
            key: fs.readFileSync('/Users/wancheng/Documents/GitHub/jessibuca-webTransport/demo/monibuca.com.key'),
        }
    }
};
export default config;
