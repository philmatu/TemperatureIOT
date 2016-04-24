<?php
$devicepassword = "CHANGEME";
$servername = "CHANGEME.com";
$username = "CHANGEME";
$password = "CHANGEME";
$dbname = "CHANGEME";

if(!isset($_GET['password'])){
	die("Sorry, this is denied!<br />\r\n\r\n");
}

if(trim($_GET['password']) != $devicepassword){
	die("Sorry, this is denied!!<br />\r\n\r\n");
}

if(isset($_GET['setup'])){
	echo "300;\r\n";//change 300 (seconds to 5 minutes) to however many seconds between uploads you need
}
// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error ."\r\n\r\n");
}

$query = "CREATE TABLE IF NOT EXISTS temperature (";
$query .= "id int(11) NOT NULL AUTO_INCREMENT,";
$query .= "device int(11) NOT NULL,";
$query .= "time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,";
$query .= "tempc DOUBLE NOT NULL,";
$query .= "tempf DOUBLE NOT NULL,";
$query .= "humidity DOUBLE NOT NULL,";
$query .= "PRIMARY KEY (ID) );";

$stmt = $conn->prepare($query);
$stmt->execute();
$stmt->close();
//insert

if(isset($_GET['device']) && isset($_GET['tempc']) && isset($_GET['tempf']) && isset($_GET['humidity'])){
	if(!is_numeric($_GET['device'])){
		die("The device specified is not numeric!\r\n");
	}
	if(!is_numeric($_GET['tempc'])){
		die("The tempc specified is not numeric!\r\n");
	}
	if(!is_numeric($_GET['tempf'])){
		die("The tempf specified is not numeric!\r\n");
	}
	if(!is_numeric($_GET['humidity'])){
		die("The humidity specified is not numeric!\r\n");
	}
	$query = "INSERT INTO temperature (device, tempc, tempf, humidity) VALUES (?, ?, ?, ?)";
	$stmt = $conn->prepare($query);
	if (!$stmt) {
        	throw new Exception($conn->error, $conn->errno);
 		die("Couldn't create the statement to insert <br />\r\n");
  	}
	$stmt->bind_param("dddd", $_GET['device'], $_GET['tempc'], $_GET['tempf'], $_GET['humidity']);
	$stmt->execute();
	$stmt->close();
	echo "Inserted device ".$_GET['device']." with tempc ".$_GET['tempc'].", tempf ".$_GET['tempf'].", and humidity ".$_GET['humidity']."<br />\r\n\r\n";
}


if(isset($_GET['view'])){
	$query = "SELECT time, device, tempc, tempf, humidity FROM temperature ORDER BY id DESC LIMIT 25";
	$stmt = $conn->prepare($query);
	$stmt->execute();
    	$stmt->bind_result($time, $device, $tempc, $tempf, $humidity);
	date_default_timezone_set('America/New_York');
	while ($stmt->fetch()) {
		$utc_ts = strtotime($time);
		$offset = date("Z");
		$local_ts = $utc_ts + $offset;
		$local_time = date("Y-m-d g:i:s A", $local_ts);
        	printf ("%s  (%d): %.2fF; %.2f%%RH<br />\r\n", $local_time, $device, $tempf, $humidity);
	}
	$stmt->close();
	echo "\r\n";
}

$conn->close();
