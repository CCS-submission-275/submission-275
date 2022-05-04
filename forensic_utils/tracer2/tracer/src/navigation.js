import React from 'react';
import 'antd/dist/antd.css';
import {Layout, Menu, Button} from 'antd';
import {Frame} from './frame.js';

import {connect} from 'react-redux';
import {update, selectFrame} from "./dataSlice";


const {Header, Content, Footer} = Layout;


class Navigation extends React.Component {

    async request() {

        const data = await fetch('/tracer.json')
            .then(response => response.json());

        const {record, replay} = data;
        // this.setState({recordData: record, replayData: replay});
        this.props.update({recordData: record, replayData: replay});
    }

    componentDidMount() {
        this.request();
    }


    render() {
        const {frames, currentFrame, currentRecordSeq, currentReplaySeq, selectFrame} = this.props;

        console.log("cur frame: ", currentFrame);

        let frameElement;
        if (currentFrame === null) {
            frameElement = "Select a frame";
        } else {
            frameElement = <Frame
                frameId={currentFrame}
                key={currentFrame}
                recordSequences={currentRecordSeq}
                replaySequences={currentReplaySeq}
            />;
        }

        return (
            <Layout>
                <Header style={{position: 'fixed', zIndex: 1, width: '100%'}}>
                    <div className="logo"/>
                    <Menu theme="dark" mode="horizontal" defaultSelectedKeys={['0']}>
                        {frames.map((element, idx) =>
                            <Menu.Item
                                onClick={() => selectFrame(element)}
                                key={idx}>{element.substring(0, 5)}</Menu.Item>
                        )}
                    </Menu>
                </Header>
                <Content className="site-layout" style={{padding: '0 3em', marginTop: 64}}>
                    <div className="site-layout-background" style={{padding: '2em', minHeight: 380}}>
                        {frameElement}
                    </div>
                </Content>
                <Footer style={{textAlign: 'center'}}>Tracer for R&R evaluation</Footer>
            </Layout>
        );
    }
}

const mapStateToProps = (state, ownProps) => {
    return {
        recordData: state.tracer.recordData,
        replayData: state.tracer.replayData,
        currentRecordSeq: state.tracer.currentRecordSeq,
        currentReplaySeq: state.tracer.currentReplaySeq,
        currentFrame: state.tracer.currentFrame,
        frames: state.tracer.frames,
        ...ownProps
    };
};

export default connect(mapStateToProps, {
    update,
    selectFrame
})(Navigation);

