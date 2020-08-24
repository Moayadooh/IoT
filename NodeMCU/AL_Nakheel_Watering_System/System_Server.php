<?php
$hostname = "127.0.0.1";
$username = "root";
$password = "root";
$dbname = "iot";

$link = mysqli_connect($hostname, $username, $password, $dbname);
if(!$link)
	die("Unable to connect".mysqli_error($link));
else
{
    if(isset($_POST['insert']))
    {
        date_default_timezone_set("Asia/Muscat");
        
        $temperature = $_POST['temp_level'];
        $soil_moisture = $_POST['soil_mois_level'];
        $water_amount = $_POST['water_amount'];
        $date_time = date("Y-m-d H:i:s");
        
		$query = "INSERT INTO records (temp_level,soil_mois_level,water_amount,date_time) VALUES ('$temperature','$soil_moisture','$water_amount','$date_time');";
		
		$result = mysqli_query($link, $query);
        if(!$result)
            die("Unable to insert".mysqli_error($link));
        else
            echo "Record Inserted";
    }
	elseif(isset($_POST['retrieve']))
    {
		$query = "SELECT * FROM records ORDER BY trend_id DESC LIMIT 5;";
        $result = mysqli_query($link, $query);
        if(!$result)
            die("Unable to retrieve".mysqli_error($link));
        else
        {
            $temperature = 0;
            $soil_moisture = 0;
			$i = 0;
            while ($row = mysqli_fetch_assoc($result))
            {
				$i++;
                $temperature += $row['temp_level'];
                $soil_moisture += $row['soil_mois_level'];
            }
			if($i==0)
				echo "No Records";
			else
			{
				$temperature /= $i;
				$soil_moisture /= $i;
				echo json_encode(array("temp_level"=>$temperature,"soil_mois_level"=>$soil_moisture));
			}
        }
    }
}
mysqli_close($link);
?>