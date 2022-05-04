import ReactDOM from 'react-dom';

import Navigation from './navigation.js';
import {Provider} from 'react-redux';
import store from './store'



ReactDOM.render(
    <Provider store={store}>
        <Navigation/>
    </Provider>,
    document.getElementById('root')
);
