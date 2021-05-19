# csvtosql
turns .csv converted Excel spreadsheets (or any csv in general) into MySQL code to create/add to a database table

1. download exe or build from source yourself
2. add exe directory to PATH
3. navigate to desired output directory

<br />

argument 0: csvtosql
argument 1: csv file location
argument 2: 'create' - adds code to create a new table; 'add' - omits some generated code if the table already exists
argument 3: the name of the table (will be the name of the generated sql file)

Currently only works with some basic data types: VARCHAR, INT, and DECIMAL.
