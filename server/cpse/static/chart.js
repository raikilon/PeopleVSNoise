(function () {
    'use strict'
  
    feather.replace()

    // let timeFormat = 'MM/DD/YYYY HH:mm';

    let time_list=[]
    let db_list=[]

    for (let entry of decibels) {
      db_list.push(entry['db'])
      time_list.push(entry['ts'])
    }

    let db_labels = [
      {"db": 20, "label":"Mosquito"},
      {"db": 30, "label":"Quiet room"},
      {"db": 60, "label":"Chatting"},
      {"db": 70, "label":"Near a motorway"},
      {"db": 80, "label":"Busy crossroads"},
      {"db": 90, "label":"Truck, Motorcycle"},
      {"db": 100, "label":"Subway"},
      {"db": 110, "label":"Car horn"},
      {"db": 120, "label":"Aircraft at take off"},
    ]
  
    // Graphs
    var ctx = document.getElementById('myChart')
    // eslint-disable-next-line no-unused-vars
    var myChart = new Chart(ctx, {
      type: 'line',
      data: {
        labels: time_list,
        // [
        //   'Sunday',
        //   'Monday',
        //   'Tuesday',
        //   'Wednesday',
        //   'Thursday',
        //   'Friday',
        //   'Saturday'
        // ],
        datasets: [{
          data: db_list,
          // [
          //   15339,
          //   21345,
          //   18483,
          //   24003,
          //   23489,
          //   24092,
          //   12034
          // ],
          lineTension: 0,
          backgroundColor: 'transparent',
          borderColor: '#007bff',
          borderWidth: 4,
          pointBackgroundColor: '#007bff'
        }]
      },
      options: {
        scales: {
					// xAxes: [{
					// 	type: 'time',
					// 	time: {
					// 		parser: timeFormat,
					// 		// round: 'day'
					// 		tooltipFormat: 'll HH:mm'
					// 	},
					// 	scaleLabel: {
					// 		display: true,
					// 		labelString: 'Date'
					// 	}
					// }],
          yAxes: [{
            ticks: {
              beginAtZero: true,
              max: 120,
              // Add the decibel label to the Y axis
              callback: function(value, index, values) {
                for (let entry of db_labels) {
                  if (entry.db === value) {
                    return value + ' ' + entry.label
                  }
                }
                return value;
            }
            },
            display: true,
            type: 'logarithmic'
          }]
        },
        legend: {
          display: false
        }
      }
    })
  }())