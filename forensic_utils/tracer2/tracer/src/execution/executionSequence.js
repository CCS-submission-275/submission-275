import {ExecutionEvent} from "./executionEvent";
import React from 'react';

export class ExecutionSequence extends React.Component {

    constructor(props) {
        super(props);
    }


    _initializeExecutionEvents() {
        // Initialize ExecutionEvents.
        this.executionEvents = [];
        for (const eventId in this.props.executionEvents) {
            const executionEvent = <ExecutionEvent
                key={eventId}
                eventId={eventId}
                eventName={this.props.executionEvents[eventId].ExecutionEvent}
                apis={this.props.executionEvents[eventId].apis}
                executionOnly={this.props.executionOnly}
                recordValues={this.props.recordValues}
                args={this.props.args}
            />;
            this.executionEvents.push(executionEvent);
        }
    }


    componentWillReceiveProps(props) {
        this.setState({
            executionOnly: props.executionOnly
        });
    }


    render() {
        this._initializeExecutionEvents();
        return (
            <table>
                <thead>
                <tr>
                    <th>~</th>
                    <th>ExecutionEvent</th>
                    <th>RetValue</th>
                    <th>API</th>
                </tr>
                </thead>
                <tbody>
                {this.executionEvents}
                </tbody>
            </table>
        );
    }
}
