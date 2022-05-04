import React from 'react';
import ReactDOM, { render } from 'react-dom';
import 'antd/dist/antd.css';
import { Collapse, Table } from 'antd';

const { Panel } = Collapse;

const columns = [
  {
    title: 'Entry',
    dataIndex: 'entry',
    key: 'entry'
  },
  {
    title: 'Operation',
    dataIndex: 'operation',
    key: 'operation'
  },
  {
    title: 'Index',
    dataIndex: 'index',
    key: 'index'
  },
  {
    title: 'Info',
    dataIndex: 'info',
    key: 'info'
  }
]


export class EditDistanceResult extends React.Component {

  render() {

    return (
      <Collapse defaultActiveKey={['1']}>
        <Panel header="Substitutions" key="1">
          <p>Execution Event: {this.props.results["ExecutionEvent"]} 
             idx: {this.props.results["idx"]}</p>
          <p>Edit Distance: {this.props.results["value"]}</p>
          <Table 
            dataSource={this.props.results["op"]} 
            columns={columns} 
            pagination={false}
          />; 
        </Panel>
      </Collapse>    
    );
  }
}