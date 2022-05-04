import ExecutionSequencePair from "./executionSequencePair";
import React from 'react';

export class Frame extends React.Component {

    _initializeSequences() {
        // Initialize Execution Sequences for this frame.
        this.execSequencesPairs = [];

        // Build execution sequence Pairs
        for(let sequenceId in this.props.recordSequences) {
            let recordSequence = this.props.recordSequences[sequenceId];
            let replaySequence = null;
            if (this.props.replaySequences !== null &&
                sequenceId < this.props.replaySequences.length) {
                replaySequence = this.props.replaySequences[sequenceId];
            }

            let execSequencePair = <ExecutionSequencePair
                key = {sequenceId}
                seqId = {sequenceId}
                recordSequence = {recordSequence}
                replaySequence = {replaySequence}
                executionOnly = {this.props.executionOnly}
                recordValues = {this.props.recordValues}
                args = {this.props.args}
            />

            this.execSequencesPairs.push(execSequencePair);
        }
    }

    componentWillReceiveProps(props){
        this.setState({
            executionOnly : props.executionOnly})
    }

    render() {
        console.log(this.props);
        this._initializeSequences();
        return (
            <React.Fragment>
                <h3>Frame: {this.props.frameId}</h3>
                {this.execSequencesPairs}
            </React.Fragment>
        );
    }
}

