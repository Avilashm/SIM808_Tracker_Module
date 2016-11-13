
<?php
$servername = "localhost";
$username = "root";
$password = "";
$db = "trackerdb";// to be added after creation of data base

/*
// -----------------------------------------------------------Create connection--------------------------------------------------------------------------*/
$conn = new mysqli($servername, $username, $password,$db);//add database name variable after creating database
// Check connection
/*if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 
else {
    echo "connected" ;
}

/*----------------------------------------------------------REQD WHEN INSERTING NEW VALUES--------------------------------------------------------
$sql = "INSERT INTO Reading(Current1, Voltage ,frequency, Phase)
VALUES ('1.1', '5.3', '5 Hz', '1.57')";
if (mysqli_query($conn, $sql)) {
    echo "New record created successfully";
} else {
    echo "Error: " . $sql . "<br>" . mysqli_error($conn);
}

<form>
  First name:<br>
  <input type="text" name="firstname"><br>
  Last name:<br>
  <input type="text" name="lastname">
</form>

/*---------------------------------------------------------REQD WHEN CREATING NEW DATABASE------------------------------------------------------------
*/// sql to create table

 $sql = "ALTER TABLE Log  ADD COLUMN Status INT(1) ";

if (mysqli_query($conn, $sql)) {
    echo "Table trackerdb created successfully";
} else 
{
    echo "Error creating table: " . mysqli_error($conn);
}

/* --------------------------------------------------------------REQD FOR FIRST TIME---------------------------------------------------------------------*/
// Create database
/*$sql = "CREATE DATABASE TrackerDB";
if ($conn->query($sql) === TRUE) {
    echo "Database created successfully";
} else {
    echo "Error creating database: " . $conn->error;
}
//----------------------------------------------------------------*/
$conn->close();
?>