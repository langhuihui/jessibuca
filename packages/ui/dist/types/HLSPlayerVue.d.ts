import { PropType, DefineComponent, ExtractPropTypes, VNode, RendererNode, RendererElement, ComponentOptionsMixin, PublicProps, ComponentProvideOptions } from 'vue';
import { HLSPlayerOptions, TimeRange } from './HLSPlayer';
declare const _default: DefineComponent<ExtractPropTypes<{
    src: {
        type: StringConstructor;
        required: false;
        default: string;
    };
    options: {
        type: PropType<HLSPlayerOptions>;
        default: () => {};
    };
    timeRanges: {
        type: PropType<TimeRange[]>;
        default: () => never[];
    };
}>, () => VNode<RendererNode, RendererElement, {
    [key: string]: any;
}>, {}, {}, {}, ComponentOptionsMixin, ComponentOptionsMixin, {}, string, PublicProps, Readonly< ExtractPropTypes<{
    src: {
        type: StringConstructor;
        required: false;
        default: string;
    };
    options: {
        type: PropType<HLSPlayerOptions>;
        default: () => {};
    };
    timeRanges: {
        type: PropType<TimeRange[]>;
        default: () => never[];
    };
}>> & Readonly<{}>, {
    timeRanges: TimeRange[];
    src: string;
    options: HLSPlayerOptions;
}, {}, {}, {}, string, ComponentProvideOptions, true, {}, any>;
export default _default;
