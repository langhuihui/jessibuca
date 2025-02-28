import { PropType, DefineComponent, ExtractPropTypes, VNode, RendererNode, RendererElement, ComponentOptionsMixin, PublicProps, ComponentProvideOptions } from 'vue';
import { TimeRange } from './TimelineBase';
export interface RangeTimelineProps {
    timeRanges: TimeRange[];
    currentTime: number;
    height?: number;
    backgroundColor?: string;
    segmentColor?: string;
    gapColor?: string;
    cursorColor?: string;
    onTimeUpdate?: (time: number) => void;
    onSeek?: (time: number) => void;
    showTimeLabel?: boolean;
}
declare const _default: DefineComponent<ExtractPropTypes<{
    timeRanges: {
        type: PropType<TimeRange[]>;
        required: true;
    };
    currentTime: {
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
    showTimeLabel: {
        type: BooleanConstructor;
        default: boolean;
    };
}>, () => VNode<RendererNode, RendererElement, {
    [key: string]: any;
}> | null, {}, {}, {}, ComponentOptionsMixin, ComponentOptionsMixin, {}, string, PublicProps, Readonly< ExtractPropTypes<{
    timeRanges: {
        type: PropType<TimeRange[]>;
        required: true;
    };
    currentTime: {
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
    showTimeLabel: {
        type: BooleanConstructor;
        default: boolean;
    };
}>> & Readonly<{}>, {
    height: number;
    backgroundColor: string;
    segmentColor: string;
    gapColor: string;
    cursorColor: string;
    showTimeLabel: boolean;
}, {}, {}, {}, string, ComponentProvideOptions, true, {}, any>;
export default _default;
