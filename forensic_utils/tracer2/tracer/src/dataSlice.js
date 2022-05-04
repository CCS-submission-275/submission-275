import {createSlice} from '@reduxjs/toolkit';



export const tracerSlice = createSlice({
    name: 'tracer',
    initialState: {
        recordData: {},
        replayData: {},
        currentRecordSeq: [],
        currentReplaySeq: [],
        currentFrame: null,
        frames: [], 
        currentSelectedRecordEvents: [],
        currentSelectedReplayEvents: [],
    },
    reducers: {
        
        updateSelectedEvents: (state, action) => {
            const {payload: {tableName, rows}} = action;
            if (tableName === "record") {
                state.currentSelectedRecordEvents = rows;
            } else if (tableName === "replay") {
                state.currentSelectedReplayEvents = rows;
            }
        },

        getSelectedEvents: (state, action) => {
            return state.currentSelectedRecordEvents;
        },

        updateReplayData: (state, action) => {
            // Redux Toolkit allows us to write "mutating" logic in reducers. It
            // doesn't actually mutate the state because it uses the Immer library,
            // which detects changes to a "draft state" and produces a brand new
            // immutable state based off those changes
            console.log(action);
            state.replayData = action.payload;
        },
        updateRecordData: (state, action) => {
            state.recordData = action.payload;
        },
        update: (state, action) => {

            state.recordData = action.payload.recordData;
            state.replayData = action.payload.replayData;
            state.frames = Object.keys(action.payload.recordData);
        },
        selectFrame: (state, action) => {
            state.currentFrame = action.payload;
            state.currentRecordSeq = state.recordData[action.payload];
            state.currentReplaySeq = state.replayData[action.payload];
        },
        updateCurrentSeq: (state, action) => {
            // console.log(action);
            const {payload: {seqId, data, tableName}} = action;
            if (tableName === 'record')
                state.currentRecordSeq[seqId].ExecutionEvents = data;
            if (tableName === 'replay')
                state.currentReplaySeq[seqId].ExecutionEvents = data;
        },
        deleteExecutionEvent: (state, action) => {
            const {payload: {seqId, eventIdx, tableName}} = action;
            if (tableName === 'record')
                state.currentRecordSeq[seqId].ExecutionEvents = [...state.currentRecordSeq[seqId].ExecutionEvents.slice(0, eventIdx),
                    ...state.currentRecordSeq[seqId].ExecutionEvents.slice(eventIdx + 1)];
            if (tableName === 'replay')
                state.currentReplaySeq[seqId].ExecutionEvents = [...state.currentReplaySeq[seqId].ExecutionEvents.slice(0, eventIdx),
                    ...state.currentReplaySeq[seqId].ExecutionEvents.slice(eventIdx + 1)];
        },
        updateExecutionEvent: (state, action) => {
            const {payload: {seqId, eventIdx, tableName, name}} = action;
            console.log(action)
            if (tableName === 'record')
                state.currentRecordSeq[seqId].ExecutionEvents[eventIdx].ExecutionEvent = name;
            if (tableName === 'replay')
                state.currentReplaySeq[seqId].ExecutionEvents[eventIdx].ExecutionEvent = name;
        }
    },
});

// Action creators are generated for each case reducer function
export const {
    updateReplayData,
    updateRecordData,
    update,
    selectFrame,
    updateCurrentSeq,
    deleteExecutionEvent,
    updateExecutionEvent,
    updateSelectedEvents,
} = tracerSlice.actions;

export default tracerSlice.reducer;
