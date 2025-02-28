import { PropType, DefineComponent, ExtractPropTypes, VNode, RendererNode, RendererElement, ComponentOptionsMixin, PublicProps, ComponentProvideOptions } from 'vue';
import { TimeRange, TimelineOptions } from './TimelineBase';
export interface TimelineProps extends TimelineOptions {
    showOriginalTimeline?: boolean;
    showMediaTimeline?: boolean;
    height?: number;
    backgroundColor?: string;
    segmentColor?: string;
    gapColor?: string;
    cursorColor?: string;
}
declare const _default: DefineComponent<ExtractPropTypes<{
    timeRanges: {
        type: PropType<TimeRange[]>;
        default: () => never[];
    };
    showOriginalTimeline: {
        type: BooleanConstructor;
        default: boolean;
    };
    showMediaTimeline: {
        type: BooleanConstructor;
        default: boolean;
    };
    height: {
        type: NumberConstructor;
        default: number;
    };
    backgroundColor: {
        type: StringConstructor;
        default: string;
    };
    segmentColor: {
        type: StringConstructor;
        default: string;
    };
    gapColor: {
        type: StringConstructor;
        default: string;
    };
    cursorColor: {
        type: StringConstructor;
        default: string;
    };
    onTimeUpdate: PropType<(time: number) => void>;
    onSeek: PropType<(time: number) => void>;
}>, () => VNode<RendererNode, RendererElement, {
    [key: string]: any;
}>, {}, {}, {}, ComponentOptionsMixin, ComponentOptionsMixin, {}, string, PublicProps, Readonly< ExtractPropTypes<{
    timeRanges: {
        type: PropType<TimeRange[]>;
        default: () => never[];
    };
    showOriginalTimeline: {
        type: BooleanConstructor;
        default: boolean;
    };
    showMediaTimeline: {
        type: BooleanConstructor;
        default: boolean;
    };
    height: {
        type: NumberConstructor;
        default: number;
    };
    backgroundColor: {
        type: StringConstructor;
        default: string;
    };
    segmentColor: {
        type: StringConstructor;
        default: string;
    };
    gapColor: {
        type: StringConstructor;
        default: string;
    };
    cursorColor: {
        type: StringConstructor;
        default: string;
    };
    onTimeUpdate: PropType<(time: number) => void>;
    onSeek: PropType<(time: number) => void>;
}>> & Readonly<{}>, {
    showOriginalTimeline: boolean;
    showMediaTimeline: boolean;
    height: number;
    backgroundColor: string;
    segmentColor: string;
    gapColor: string;
    cursorColor: string;
    timeRanges: TimeRange[];
}, {}, {}, {}, string, ComponentProvideOptions, true, {}, any>;
export default _default;
