/**
 * @type {import('vite').UserConfig}
 */
import fs from 'fs';
const config = {
    server: {
        host: '0.0.0.0',
        https: {
            cert: fs.readFileSync('/Users/dexter/m7s4/monibuca/monibuca.com_bundle.pem'),
            key: fs.readFileSync('/Users/dexter/m7s4/monibuca/monibuca.com.key'),
        }
    }
};
export default config;
