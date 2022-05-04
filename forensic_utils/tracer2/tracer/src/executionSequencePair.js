import {Col, Row, Button, Collapse, Table, Space} from "antd";
import {NestedTable} from "./nestedTable";
import React from 'react';
import { shallowEqual, useSelector } from 'react-redux';
import {connect} from 'react-redux';

import {EditDistance} from "./editDistance";
import { EditDistanceResult } from "./editDistanceResult";

const { Panel } = Collapse;

const RowColors = [
    'red', 'pink', 'green'
]

const columns = [
    {
        title: 'Entry',
        dataIndex: 'entry',
        key: 'entry',
    },
    {
        title: 'Operation',
        dataIndex: 'operation',
        key: 'operation',
    },
    {
        title: 'Index',
        dataIndex: 'index',
        key: 'index',
    },
    {
        title: 'Info',
        dataIndex: 'info',
        key: 'info',
    },
];
  

class ExecutionSequencePair extends React.Component {

    constructor(props) {
        super(props)
        this.state = {
            editDistanceResults: false,
        }
    }

    componentWillReceiveProps(props) {
        this.setState({
            executionOnly: props.executionOnly,
        });
    }

    _executionEventID(record, replay) {
        let score = 0
        if (!record || !replay) {
            return RowColors[score];
        }
        if (record.apis.length === replay.apis.length) {
            score++
        }
        if (record.ExecutionEvent === replay.ExecutionEvent) {
            score++
        }
        return RowColors[score];
    }

    _colorCode() {
        const recordEventCount = this.props.recordSequence.ExecutionEvents.length;
        const replayEventCount = this.props.replaySequence.ExecutionEvents.length;
        const recordColors = {};
        const replayColors = {};
        const m = Math.min(recordEventCount, replayEventCount);
        for (let i = 0; i < m; i++) {
            const color = this._executionEventID(
                this.props.recordSequence.ExecutionEvents[i], 
                this.props.replaySequence.ExecutionEvents[i]);
            replayColors[i] = color;
            recordColors[i] = color;

        }
        return {recordColors, replayColors}
    }

    calcEditDistanceEE() {
        const editDistance = new EditDistance(
            this.props.recordSequence, this.props.replaySequence);
        this.results = [editDistance.calculateExecutionEventDistance()]
        this.setState({editDistanceResults: true});
    }


    _getAPIsfromEE(EEIndexes, events) {
        let eventInfo = [];
        for (let idx of EEIndexes) {
            eventInfo.push({
                "name" : events[idx].ExecutionEvent,
                "apis" : events[idx].apis.map(
                    (api) => "(" + api[0] + ")" + api[1])
            });
        }
        return eventInfo;
    }

    calcEditDistanceAPIs() {
        const recordApis = this._getAPIsfromEE(
            this.props.recordEvents, 
            this.props.recordSequence.ExecutionEvents
        );

        console.log(recordApis.length);

        const replayApis = this._getAPIsfromEE(
            this.props.replayEvents,
            this.props.replaySequence.ExecutionEvents
        );

        console.log("Calculating edit distance for apis");

        this.results = [];
        for (let idx in recordApis) {
            let editDistance = new EditDistance(
                recordApis[idx]["apis"], replayApis[idx]["apis"]);
            let curResult = editDistance.calculateEditDistanceFromAPIs();
            if (curResult.value != 0) {
                curResult["ExecutionEvent"] = recordApis[idx]['name'];
                curResult["idx"] = idx;
                this.results.push(curResult);
            }
        }

        this.setState({editDistanceResults: true});
    }

    render() {
        // this._initializeExecutionSequences();
        const {
            recordColors,
            replayColors
        } = this._colorCode();
        const seqId = Number(this.props.seqId);

        let editDistanceResultElements;
        if (this.state.editDistanceResults) {
            editDistanceResultElements = [];
            for (let result of this.results) {
                let editDistanceResultElement = <EditDistanceResult
                    results = {result}/>;
                editDistanceResultElements.push(editDistanceResultElement);
            }
        } else {
            editDistanceResultElements = <div/>;
        }

        return (
            <>
            <Space>
                <Button onClick={this.calcEditDistanceEE.bind(this)} type="primary">Edit Distance EE</Button> 
                <Button onClick={this.calcEditDistanceAPIs.bind(this)} type="primary">Edit Distance APIs</Button>
            </Space>
            {editDistanceResultElements}
            <Row justify="space-between">
                <Col span={11}>
                    <NestedTable
                        executionEvents={this.props.recordSequence.ExecutionEvents}
                        rowColors={recordColors}
                        tableName="record"
                        seqId={seqId}
                    />
                </Col>
                <Col span={11}>
                    <NestedTable
                        executionEvents={this.props.replaySequence.ExecutionEvents}
                        rowColors={replayColors}
                        tableName="replay"
                        seqId={seqId}
                    />
                </Col>
            </Row>
            </>
        );
    }
}


const mapStateToProps = (state, ownProps) => {
    return {
        recordEvents: state.tracer.currentSelectedRecordEvents,
        replayEvents: state.tracer.currentSelectedReplayEvents,
        ...ownProps
    };
};

export default connect(mapStateToProps, {
})(ExecutionSequencePair);
