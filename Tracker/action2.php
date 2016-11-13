<?php
date_default_timezone_set("Asia/Kolkata"); 

$GPGGA= (isset($_GET['GPGGA']) ? $_GET['GPGGA'] : null);

$servername = "localhost"; 
$username = "thingsdataroot";
$password = "pa1303092$";
$db = "thingsdata";

$conn = new mysqli($servername, $username, $password,$db);


if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 
else {
    // echo " Connected   " ;
}
if(strlen($GPGGA) == 72) {

$Time=substr($GPGGA,7,10);
$x=substr($GPGGA,18,9);
$xD=substr($GPGGA,28,1);
$y=substr($GPGGA,30,10);
$yD=substr($GPGGA,41,1);
//*******************************************************CONVERTING NMEA TO DECIMAL VALUES************************************************

$x_pre= (float)substr($x,0,2);
$x_post =(float)substr($x,2);
$y_pre= (float)substr($y,0,3);
$y_post =(float)substr($y,3);
$x_new = $x_pre + $x_post/60;
$y_new = $y_pre + $y_post/60;
$x_new = (string)$x_new;
$y_new = (string)$y_new;
//echo $x_new."<br>",$y_new;

//****************************************************************************************************************************************
$x = $x_new;
$y = $y_new;
$x = $x."-".$xD;
$y = $y."-".$yD;
//Function to check if (x, y) belongs to region

/*$jsonItem=array();
$jsonarray=array();
$jsonItem['x1']=2850; 
$jsonItem['x2']=2850; 
$jsonItem['x3']=2800; 
$jsonItem['x4']=2800; 
$jsonItem['y1']=7680; 
$jsonItem['y2']=7680; 
$jsonItem['y3']=7720; 
$jsonItem['y4']=7720; 
$jsonItem['phno.']="+918700971763";
array_push($jsonarray,$jsonItem);  	
echo json_encode($jsonarray);	*/
/*
$decodedjson = fread($myfile,filesize("data.json"));
$decodemsg = json_decode($decodedjson);
*/

$myfile = fopen("data.json", "r");
$decodedjson = fread($myfile,filesize("data.json"));
fclose($myfile);
//echo $decodedjson;
$json = substr($decodedjson,1,145);  //Removing Sqaure brackets
$result = json_decode($json, true);	 //Decoding and parsing JSON
//var_dump($result);
$x1 =  $result['x1'];
$x2 =  $result['x2'];
$x3 =  $result['x3'];
$x4 =  $result['x4'];
$y1 =  $result['y1'];
$y2 =  $result['y2'];
$y3 =  $result['y3'];
$y4 =  $result['y4'];
$phno = $result['phno.'];

//echo decodedjson = $decodedjson.'<br>';
//echo $x.'<br>';
//echo $y.'<br>';
//****************************************************************Checks current Device Status*****************************************************************

	$x_in_region = 0;
	$y_in_region = 0;
	
	if (($x <= $x1 || $x <= $x2 || $x <= $x3 || $x <= $x4 ) && ($x >= $x1 || $x >= $x2 || $x >= $x3 || $x >= $x4))
	{
	    $x_in_region = 1;
		//echo "x_in_region = ". $x_in_region."<br>" ;
	}
	if (($y <= $y1 || $y <= $y2 || $y <= $y3 || $y <= $y4 ) && ($y >= $y1 || $y >= $y2 || $y >= $y3 || $y >= $y4))
	{
	    $y_in_region = 1;
		//echo " y_in_region = ". $y_in_region."<br>" ;
	}
	
	if ($x_in_region == 1 && $y_in_region == 1)
	{  
		$current_pos = 0;  //Already in region
		//echo "Current_pos = ".$current_pos."<br>" ;
	}
	else
	{
	   $current_pos = 1;	//Currently out of region
	  // echo "Current_pos = ".$current_pos."<br>" ;
    }	
//echo $current_pos . '<br>';
	//$current_pos = 0;
	
//*************************************************************************************************************************************************************
$sql = "SELECT status FROM log ORDER BY reg_date DESC LIMIT 1";			//Selects The last entered value
if (mysqli_query($conn, $sql)) {
   // echo "selected Top";
} else {
    echo "Error: " . $sql . "<br>" . mysqli_error($conn);
}
				$result = $conn->query($sql);
				if ($result->num_rows > 0) {			//Checks if resulting no. of rows is greater than 0
					while($row = $result->fetch_assoc()) 
						{
							$pre_pos = $row["status"];
						//echo "pre_pos = ".$pre_pos."<br>";
						}
				}
	if($pre_pos > $current_pos) {
		$msg_string = "\$\"". $phno. "\"Entering The AreaZ";			//Fires when Device enter the Specified area.
	}					
	elseif($pre_pos < $current_pos) {
		$msg_string = "\$\"". $phno. "\"Leaving The AreaZ";				//Fires when Device Leaves the specified area. 
	
	}
	else{
		$msg_string = "\$No changeZ";									//When Device has the same location status as earlier.
	}

$sql = "SELECT * FROM log ORDER BY reg_date ASC"; 			//Re- Ordering the table

if (mysqli_query($conn, $sql)) {
    //echo "Value Inserted";
} else {
    echo "Error: " . $sql . "<br>" . mysqli_error($conn);
}



$sql = "INSERT INTO `log`(`id`, `Latitude`, `Longitude`, `reg_date`,`status`) VALUES (NULL, '$x', '$y', CURRENT_TIMESTAMP, $current_pos);"; //Inserts New GPS value in the Database
if (mysqli_query($conn, $sql)) {
   echo $msg_string;
} else {
    echo "Error: " . $sql . "<br>" . mysqli_error($conn);
}
						
}

else echo "\$Invalid StringZ";			//Fires when recieves an invlaid String 

?>
