window.addEventListener('DOMContentLoaded', styleTables);


var styleDict = new Object();
var colors = ["orange", "red", "green", "blue", "pink", "purple", "khaki", 
              "light-blue"];
var cur_color = 0;

function styleTables(event) {
  var tables = document.body.querySelectorAll('table')
  tables.forEach((table) => {styleTable(table)})
  // Style Grids.
  var grids = document.body.querySelectorAll('.grid-container')
  grids.forEach((grid) => {styleGrid(grid)})
}


function styleGrid(grid) {
  var record_table = grid.querySelector('.record-table')
  var replay_table = grid.querySelector('.replay-table')

  // We filter out all td elements that are not APIs using the styleDict,
  // where keys in the styleDict are the names of execution events.
  var tds = record_table.querySelectorAll('td')
  

  var record_apis = [...tds].filter((td) => !(td.textContent in styleDict))
  record_apis.forEach((td) => {console.log(td.textContent);})
  console.log(record_apis.length)
  //var record_apis = [...tds].filter((td) => !(td.textContent in styleDict))
}

function styleTable(table) {
  rows = table.querySelectorAll('tr')
  rows.forEach((row) => { styleRow(row);})
}

function styleRow(row) {
    let eventTd = row.querySelector('td');
    if (eventTd == null) return;

    if (!(eventTd.textContent in styleDict)) {
      var color = colors[cur_color]
      cur_color = (cur_color + 1) % colors.length
      styleDict[eventTd.textContent] = colors[cur_color]
    }

    eventTd.setAttribute('class', styleDict[eventTd.textContent])
}







