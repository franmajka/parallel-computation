export enum RequestMethod {
  SET_DATA,
  START_PROCESSING,
  GET_RESULT,
  END,
};

export enum Status {
  CONNECTION_ESTABLISHED,
  DATA_RECEIVED,
  RESULT_IN_PROGRESS,
  SUCCESS,
  ERROR,
};
