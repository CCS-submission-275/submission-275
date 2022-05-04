export function APIRow(props){
    return (
        <tr onClick = {props.onClick}>
            <td> {props.apiId} </td>
            <td bgcolor = {props.color}>{props.eventName}</td>
            <td>{props.retValue}</td>
            <td>{props.name}</td>
        </tr>
    );
}
