<?php
    $hostname = "127.0.0.1"; //servername
	$username = "root";
	$password = "root";	
	$dbname = "nodemcu";

    $conn = new mysqli($hostname, $username, $password, $dbname);
    if ($conn->connect_error) {
        die("Database Connection failed: ".$conn->connect_error);
    }
    else
	{
		if(isset($_POST['insert']))
		{
			$temperature = $_POST['temp'];
			$soil_moisture = $_POST['soil_mo'];
			date_default_timezone_set("Asia/Muscat");
			$date_time = date("Y-m-d H:i:s");
			
			$sql = "INSERT INTO trends (Temp_Level,Mois_Level,Date_Time) VALUES ('$temperature','$soil_moisture','$date_time');";
			
			if ($conn->query($sql) === TRUE) {
				echo json_encode(array("temp"=>$temperature,"soil_mo"=>$soil_moisture,"date_time"=>$date_time));
			} else {
				echo "Error: ".$sql."<br>".$conn->error;
			}
		}
	}
	$conn->close();
?>