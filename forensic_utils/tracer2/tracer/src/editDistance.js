
export class EditDistance {

    constructor(record, replay) {
        // The record and replay execution events.
        this.record = record;
        this.replay = replay;
    }

    _getExecutionEvents(sequence) {
        let executionEvents = [];
        for (let i = 0; i < sequence.ExecutionEvents.length; i++) {
            executionEvents.push(sequence.ExecutionEvents[i].ExecutionEvent);
        }
        return executionEvents;
    }

    calculateExecutionEventDistance() {
        // Get execution events. 
        let recordSequence = this._getExecutionEvents(this.record);
        let replaySequence = this._getExecutionEvents(this.replay);
        let result = this.calculate(recordSequence, replaySequence)
        result["op"] = result["op"].map((e, idx) => {
            return {
                "key": idx,
                "entry": idx,
                "operation": e[0], 
                "info": e[1],  
                "index": e[2]}
        });

        return result;
    }

    calculateEditDistanceFromAPIs() {
        let result = this.calculate(this.record, this.replay)
        result["op"] = result["op"].map((e, idx) => {
            return {
                "key": idx,
                "entry": idx,
                "operation": e[0], 
                "info": e[1],  
                "index": e[2]}
        });
        return result;
    }

    calculateOld(sequence1, sequence2) {
        //BUG(joey): recursion + memo approach leads to chrome and 
        // firefox's recursion limit. 
        function helper(idx1, idx2) {    
            // Check if value is in cache. 
            if ([idx1, idx2] in cache) {
                return cache[[idx1, idx2]];
            }
            
            let result;
            if (idx1 === m) {
                result =  {"value": n - idx2, 
                    "op": [['i', sequence2.slice(idx2, n), idx2]]}
            } else if (idx2 === n) {
                result = {"value": m - idx1, 
                    "op": [['r', sequence1.slice(idx1, m)], idx1]}
            } else if (sequence1[idx1] === sequence2[idx2]) {
                result = helper(idx1 + 1, idx2 + 1);
            } else {
                const remove = helper(idx1 + 1, idx2);
                const insert = helper(idx1, idx2 + 1);
                const sub = helper(idx1 + 1, idx2 + 1);
                result = Math.min(remove["value"], insert["value"], sub["value"]);
                
                let op, prev_ops;
                if (result ===  sub["value"]) {
                    op = [["sub", sequence1[idx1] + "->" + sequence2[idx2], idx2]]
                    prev_ops = sub["op"];
                } else if (result === remove["value"]) {
                    op = [["remove", sequence1[idx1], idx1]]
                    prev_ops = remove["op"]
                } else if (result === insert["value"]) {
                    op =  [["insert", sequence2[idx2], idx2]]
                    prev_ops = insert["op"];
                } else {
                    console.assert(false);
                }
                
                result = {
                    "value": result + 1,
                    "op": op.concat(prev_ops)
                };
            }
            
            cache[[idx1, idx2]] = result;
            return result;
        }

        let cache = new Map();
        const m = sequence1.length;
        const n = sequence2.length; 
        return helper(0, 0);
    }

    extractOperations(sequence1, sequence2, dp) {
        
        const m = dp.length;
        const n = dp[0].length;
        let operations = [];
        
        let row = m - 1;
        let col = n - 1;
        while (true) {
            let sub, insert, remove;

            // if row and col == 0, then we are finished. 
            if (row == 0 && col == 0) 
                break;
            
            // Get sub value, if sub was possible. 
            if (row == 0 || col == 0)
                sub = Infinity
            else
                sub = dp[row - 1][col - 1];
            
            // Get insert value, if insert was possible. 
            insert = Infinity
            if (col != 0)
                insert = dp[row][col - 1];
            
            // Get remove value, if remove was possible. 
            remove = Infinity
            if (row != 0)
                remove = dp[row - 1][col];
            
            // Determine which operation will lead to the min. edit. 
            const op = Math.min(sub, remove, insert);
            
            // If the op and current edit distance is the same, then
            // no edit was needed here. 
            const isSame = (op === dp[row][col])
            
            // Record the operation & update row and col values based
            // on which operation was selected. 
            let operation;
            if (op === sub) {
                operation = ["sub", sequence1[row] + "<-->" + sequence2[col], m - row - 1];
                row = row - 1;
                col = col - 1;
            } else if (op === remove) {
                operation = ["remove", sequence1[row], m - row - 1];
                row = row - 1;
                col = col;
                
        } else if (op === insert) {
            operation = ["insert", sequence2[col], n - col - 1];
            row = row;
            col = col - 1;
        }
        
            if (!isSame) 
                operations.push(operation); 
        }
        
        return operations
    }


    calculate(sequence1, sequence2) {
        sequence1.push('#');
        sequence2.push('#');
        sequence1 = sequence1.reverse()
        sequence2 = sequence2.reverse()
        const m = sequence1.length;
        const n = sequence2.length;
        
        let dp = new Array(m).fill(0).map(() => new Array(n).fill(0));
        
        // Initialize dp.
        for (let idx = 0; idx < m; idx++)
            dp[idx][0] = idx;
        for (let idx = 0; idx < n; idx++)
            dp[0][idx] = idx;
        
        for (let row = 1; row < m; row++) {
            for (let col = 1; col < n; col++) {
                let sub;
                if (sequence1[row] === sequence2[col])
                    sub = dp[row - 1][col - 1] - 1;
                else
                    sub = dp[row - 1][col - 1];
                
                dp[row][col] = 1 + Math.min(dp[row - 1][col], dp[row][col - 1], sub);
            }
        }

        const result = {
            "value" : dp[m - 1][n - 1],
            "op" : this.extractOperations(sequence1, sequence2, dp)
        }
        return result;
    }
}