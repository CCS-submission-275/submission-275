import { configureStore } from '@reduxjs/toolkit'
import tracerReducer from './dataSlice';
export default configureStore({
    reducer: {
        tracer: tracerReducer
    },
})
