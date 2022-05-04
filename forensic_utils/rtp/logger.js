const {createLogger, format, transports} = require('winston');
const {combine, printf, timestamp, align, colorize} = format;

function getConsoleLogger(module) {
  return createLogger({
    level: 'debug',
    format: combine(
      colorize(),
      timestamp(),
      align(),
      printf(({
                message,
                level,
                timestamp,
              }) => `${timestamp}::${module}::${level}: ${message}`),
    ),
    transports: [
      new transports.Console()
    ]
  });
}

module.exports = {
  getConsoleLogger
};
