import * as utils from "../utils";
import {APIRow} from "./apiRow";
import React from 'react';

const colors = ['#008080', '#FA8072', 'E4BCB4', '#808000',
    'B6d3D0', '85F3EA', 'F3D085', '85F385'];

const colorMap = new Map();


export class ExecutionEvent extends React.Component {

    constructor(props) {

        super(props);
        // Initialize APIs.

        // Select the color for this execution event.
        if (!(this.props.eventName in colorMap)) {
            const idx = Math.floor(Math.random() * colors.length);
            colorMap[this.props.eventName] = colors[idx];
        }

        this.color = colorMap[this.props.eventName];
        this.apiArray = [];
        for (const apiId in props.apis) {

            let retValue = this.props.apis[apiId][0];
            if (retValue.length > 50) {
                retValue = "hash-" + utils.hashCode(retValue);
            }

            let api = <APIRow
                key={apiId}
                apiId={apiId}
                color={this.color}
                eventName={this.props.eventName}
                retValue={retValue}
                name={this.props.apis[apiId][1]}
                args={this.props.apis[apiId][2]}
                onClick={this.handleClick.bind(this)}
            />;
            this.apiArray.push(api);
        }
        this.state = {
            executionOnly: this.props.executionOnly
        };
    }

    handleClick() {
        this.setState(({executionOnly}) => ({executionOnly: !executionOnly}));
    }

    componentWillReceiveProps(props) {
        this.setState({
            executionOnly: props.executionOnly
        });
    }


    render() {

        if (this.apiArray.length <= 1) {
            return <React.Fragment/>;
        }

        if (this.state.executionOnly) {
            return (
                <React.Fragment>
                    <tr onClick={this.handleClick.bind(this)}>
                        <td>{this.props.apis.length}</td>
                        <td bgcolor={this.color}>{this.props.eventName}</td>
                    </tr>
                </React.Fragment>
            );
        } else {
            return (
                <React.Fragment>
                    {this.apiArray}
                </React.Fragment>
            );
        }
    }
}
