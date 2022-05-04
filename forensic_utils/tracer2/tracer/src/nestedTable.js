import React, {useRef, useCallback, useContext, useState, useEffect} from 'react';

import 'antd/dist/antd.css';
import {Table, Form, Input, Popconfirm, Checkbox} from 'antd';

import {DndProvider, useDrag, useDrop} from 'react-dnd';
import {HTML5Backend} from 'react-dnd-html5-backend';

import update from 'immutability-helper';

import {useDispatch} from 'react-redux';
import {updateCurrentSeq, deleteExecutionEvent, updateExecutionEvent, 
        updateSelectedEvents} from "./dataSlice";

const type = 'DraggableBodyRow';

const EditableContext = React.createContext(null);


const EditableCell = ({
                          title,
                          editable,
                          children,
                          dataIndex,
                          record,
                          handleSave,
                          ...restProps
                      }) => {
    const [editing, setEditing] = useState(false);
    const inputRef = useRef(null);
    const form = useContext(EditableContext);
    useEffect(() => {
        if (editing) {
            inputRef.current.focus();
        }
    }, [editing]);

    const toggleEdit = () => {
        setEditing(!editing);
        form.setFieldsValue({
            [dataIndex]: record[dataIndex],
        });
    };

    const save = async () => {
        try {
            const values = await form.validateFields();
            toggleEdit();
            handleSave({...record, ...values});
        } catch (errInfo) {
            console.log('Save failed:', errInfo);
        }
    };

    let childNode = children;

    if (editable) {
        childNode = editing ? (
            <Form.Item
                style={{
                    margin: 0,
                }}
                name={dataIndex}
                rules={[
                    {
                        required: true,
                        message: `${title} is required.`,
                    },
                ]}
            >
                <Input ref={inputRef} onPressEnter={save} onBlur={save}/>
            </Form.Item>
        ) : (
            <div
                className="editable-cell-value-wrap"
                style={{
                    paddingRight: 24,
                }}
                onClick={toggleEdit}
            >
                {children}
            </div>
        );
    }
    return <td {...restProps}>{childNode}</td>;
};

const DraggableBodyRow = ({index, moveRow, className, style, ...restProps}) => {
    const ref = useRef();
    const [{isOver, dropClassName}, drop] = useDrop({
        accept: type,
        collect: monitor => {
            const {index: dragIndex} = monitor.getItem() || {};
            if (dragIndex === index) {
                return {};
            }
            return {
                isOver: monitor.isOver(),
                dropClassName: dragIndex < index ? ' drop-over-downward' : ' drop-over-upward',
            };
        },
        drop: item => {
            moveRow(item.index, index);
        },
    });
    const [, drag] = useDrag({
        type,
        item: {index},
        collect: monitor => ({
            isDragging: monitor.isDragging(),
        }),
    });
    drop(drag(ref));

    const [form] = Form.useForm();
    return (
        <Form form={form} component={false}>
            <EditableContext.Provider value={form}>
                <tr
                    ref={ref}
                    className={`${className}${isOver ? dropClassName : ''}`}
                    style={{cursor: 'move', ...style}}
                    {...restProps}
                />
            </EditableContext.Provider>
        </Form>
    );
};

export function NestedTable(props) {
    const dispatch = useDispatch();
    const expandedRowRender = (props) => {
        const columns = [
            {title: 'entry', dataIndex: 'entry', key: 'entry'},
            {title: 'api', dataIndex: 'api', key: 'api'},
            {title: 'Return Value', dataIndex: 'retValue', key: 'retValue'},
            {title: 'Arguments', dataIndex: 'arguments', key: 'arguments'},
        ];

        // console.log("props:", props);
        const data = [];
        for (let i = 0; i < props.executionEvent.apis.length; ++i) {
            data.push({
                key: i,
                entry: i,
                retValue: props.executionEvent.apis[i][0],
                api: props.executionEvent.apis[i][1],
                arguments: props.executionEvent.apis[i][2],
            });
        }
        return (
            <Table 
                columns={columns} 
                dataSource={data} 
                pagination={false}
            />);
    };
    const {seqId, tableName} = props;
    let columns = [
        {title: 'entry', dataIndex: 'entry', key: 'entry'},
        {title: 'count', dataIndex: 'count', key: 'count'},
        {title: 'ExecutionEvent', dataIndex: 'name', key: 'name', editable: true},
        {
            title: 'action',
            dataIndex: 'action',
            disabled : true,
            render: (_, record, eventIdx) =>

                <a onClick={() => dispatch(deleteExecutionEvent({tableName, seqId, eventIdx}))}> 
                    Delete
                </a>
        }
    ];

    columns = columns.map((col) => {
        if (!col.editable) {
            return col;
        }

        return {
            ...col,
            onCell: (record, eventIdx) => ({
                record,
                editable: col.editable,
                dataIndex: col.dataIndex,
                title: col.title,
                handleSave: ({name}) => {dispatch(updateExecutionEvent({tableName, seqId, eventIdx, name }))},
            }),
        };
    });

    const data = [];
    for (let i = 0; i < props.executionEvents.length; ++i) {
        data.push({
            key: i,
            entry: i,
            count: props.executionEvents[i].apis.length,
            name: props.executionEvents[i].ExecutionEvent,
            executionEvent: props.executionEvents[i],
        });
    }

    const components = {
        body: {
            row: DraggableBodyRow,
            cell: EditableCell
        }
    };
    const moveRow = useCallback(
        (dragIndex, hoverIndex) => {
            const dragRow = props.executionEvents[dragIndex];
            dispatch(updateCurrentSeq({
                    tableName, seqId, data:
                        update(props.executionEvents, {
                            $splice: [
                                [dragIndex, 1],
                                [hoverIndex, 0, dragRow],
                            ],
                        })
                }),
            );
        },
        [props.executionEvents, tableName, seqId],
    );

    const selectRow = function(rows) {
        dispatch(updateSelectedEvents({
            "tableName": props.tableName, 
            rows}));
    }

    return (
        <DndProvider backend={HTML5Backend}>
            <Table
                className="components-table-demo-nested"
                columns={columns}
                expandable={{expandedRowRender}}
                dataSource={data}
                pagination={false}
                components={components}
                rowClassName={(record, index) => {
                    if (props.rowColors && props.rowColors.hasOwnProperty(index)) {
                        return props.rowColors[index];
                    } else return '';
                }}
                onRow={(record, index) => ({
                    index,
                    moveRow,
                })}
                rowSelection={{
                    onChange: selectRow,
                }}

            />
        </DndProvider>
    );
}

