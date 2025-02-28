import { TimelineBaseProps } from './TimelineBase';
import { DefineComponent, ExtractPropTypes, VNode, RendererNode, RendererElement, ComponentOptionsMixin, PublicProps, ComponentProvideOptions } from 'vue';
export interface BasicTimelineProps extends TimelineBaseProps {
    duration: number;
    progressColor?: string;
}
declare const _default: DefineComponent<ExtractPropTypes<{
    currentTime: {
        type: NumberConstructor;
        required: true;
    };
    duration: {
        type: NumberConstructor;
        required: true;
    };
    height: {
        type: NumberConstructor;
        default: number;
    };
    backgroundColor: {
        type: StringConstructor;
        default: string;
    };
    progressColor: {
        type: StringConstructor;
        default: string;
    };
    cursorColor: {
        type: StringConstructor;
        default: string;
    };
}>, () => VNode<RendererNode, RendererElement, {
    [key: string]: any;
}>, {}, {}, {}, ComponentOptionsMixin, ComponentOptionsMixin, {}, string, PublicProps, Readonly< ExtractPropTypes<{
    currentTime: {
        type: NumberConstructor;
        required: true;
    };
    duration: {
        type: NumberConstructor;
        required: true;
    };
    height: {
        type: NumberConstructor;
        default: number;
    };
    backgroundColor: {
        type: StringConstructor;
        default: string;
    };
    progressColor: {
        type: StringConstructor;
        default: string;
    };
    cursorColor: {
        type: StringConstructor;
        default: string;
    };
}>> & Readonly<{}>, {
    height: number;
    backgroundColor: string;
    cursorColor: string;
    progressColor: string;
}, {}, {}, {}, string, ComponentProvideOptions, true, {}, any>;
export default _default;
