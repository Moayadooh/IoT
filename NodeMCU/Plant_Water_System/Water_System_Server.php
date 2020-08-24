<?php
$hostname = $_POST['hostname'];
$username = $_POST['username'];
$password = $_POST['password'];
$dbname = $_POST['dbname'];

$link = mysqli_connect($hostname, $username, $password, $dbname);
if(!$link)
	die("Unable to connect".mysqli_error($link));
else
{
    if(isset($_POST['COMPUTE_WATER_AMOUNT_CBR']))
    {
        date_default_timezone_set("Asia/Muscat");
        
        $temperature = $_POST['temp_level'];
        $soil_moisture = $_POST['soil_mois_level'];
		$plant_id = $_POST['plant_id'];
        $date_time = date("Y-m-d H:i:s");
		
		$query = "select plant_name as name from plants where plant_id = '$plant_id';";
		$result = mysqli_query($link, $query);
        if(!$result)
            die("Unable to retrieve ".mysqli_error($link));
		$row = mysqli_fetch_assoc($result);
		$plant_name = $row['name'];
		$plant_name = strtolower($plant_name);
        
		//Check if the problem is exist 'RETRIEVE'
		$query = "select COUNT(*) as total, water_amount as amount from ".$plant_name." where temp_level = '".$temperature."' and soil_mois_level = '".$soil_moisture."';";
		$result = mysqli_query($link, $query);
        if(!$result)
            die("Unable to retrieve ".mysqli_error($link));
        $row = mysqli_fetch_assoc($result);
		$total = $row['total'];
		if($total>0)
		{
			$water_amount = $row['amount']; //REUSE
			$query = "INSERT INTO trends (plant_id,temp_level,soil_mois_level,water_amount,sensors_status,date_time) VALUES ('$plant_id','$temperature','$soil_moisture','$water_amount','ON','$date_time');";
			$result = mysqli_query($link, $query);
			if(!$result)
				die("Unable to insert ".mysqli_error($link));
			echo json_encode(array("water_amount"=>$water_amount));
		}
		else //REVISE
		{
			$query = "select min(temp_level) as min_temp from ".$plant_name.";";
			$result = mysqli_query($link, $query);
			if(!$result)
				die("Unable to retrieve ".mysqli_error($link));
			$row = mysqli_fetch_assoc($result);
			$minTempLevel = $row['min_temp'];
			
			$query = "select max(temp_level) as max_temp from ".$plant_name.";";
			$result = mysqli_query($link, $query);
			if(!$result)
				die("Unable to retrieve ".mysqli_error($link));
			$row = mysqli_fetch_assoc($result);
			$maxTempLevel = $row['max_temp'];
			
			$rangeTempLevel = $maxTempLevel - $minTempLevel;
			
			$query = "select min(soil_mois_level) as min_mois from ".$plant_name.";";
			$result = mysqli_query($link, $query);
			if(!$result)
				die("Unable to retrieve ".mysqli_error($link));
			$row = mysqli_fetch_assoc($result);
			$minSoilMoisLevel = $row['min_mois'];
			
			$query = "select max(soil_mois_level) as max_mois from ".$plant_name.";";
			$result = mysqli_query($link, $query);
			if(!$result)
				die("Unable to retrieve ".mysqli_error($link));
			$row = mysqli_fetch_assoc($result);
			$maxSoilMoisLevel = $row['max_mois'];
			
			$rangeSoilMoisLevel = $maxSoilMoisLevel - $minSoilMoisLevel;
			
			function LocalSimilarityContinuous($input, $retrievedData, $range)
			{
				$sum = 1 - abs(($input - $retrievedData)) / $range;
				return $sum;
			}
			
			function GlobalSimilarity($tempLevel, $soilMoisLevel)
			{
				$sum = 0.5 * ((1 * $tempLevel) + (1 * $soilMoisLevel));
				return $sum;
			}
			
			$query = "select * from ".$plant_name.";";
			$result = mysqli_query($link, $query);
			if(!$result)
				die("Unable to retrieve ".mysqli_error($link));
			$highestGS = 0;
			while ($row = mysqli_fetch_assoc($result))
			{
				$lsTemp = LocalSimilarityContinuous($temperature, $row['temp_level'], $rangeTempLevel);
				$lsSoilMois = LocalSimilarityContinuous($soil_moisture, $row['soil_mois_level'], $rangeSoilMoisLevel);
				$GS = GlobalSimilarity($lsTemp, $lsSoilMois);
				
				if($GS>$highestGS)
				{
					$highestGS = $GS;
					$water_amount = $row['water_amount'];
				}
			}
			$water_amount *= $highestGS;
			
			//RETAIN
			$query = "INSERT INTO trends (plant_id,temp_level,soil_mois_level,water_amount,sensors_status,date_time) VALUES ('$plant_id','$temperature','$soil_moisture','$water_amount','ON','$date_time');";
			$result = mysqli_query($link, $query);
			if(!$result)
				die("Unable to insert ".mysqli_error($link));
			echo json_encode(array("water_amount"=>$water_amount));
		}
    }
	elseif(isset($_POST['COMPUTE_WATER_AMOUNT_MLR']))
    {
		$plant_id = $_POST['plant_id'];
        $date_time = date("Y-m-d H:i:s");
		
		$query = "SELECT temp_level, soil_mois_level, water_amount FROM trends where plant_id = '$plant_id' and sensors_status = 'ON';";
        $result = mysqli_query($link, $query);
        if(!$result)
            die("Unable to retrieve ".mysqli_error($link));
        else
        {
			$N = 0;
			
			$x1 = 0;
			$x2 = 0;
			$y = 0;
			
			$x1Total = 0;
			$x2Total = 0;
			$yTotal = 0;
			
			$x1SquareTotal = 0;
			$x2SquareTotal = 0;
			
			$x1y = 0;
			$x2y = 0;
			$x1x2 = 0;
            while ($row = mysqli_fetch_assoc($result))
            {
				$N++;
				$x1 = $row['temp_level'];
				$x2 = $row['soil_mois_level'];
				$y = $row['water_amount'];
				
				$x1Total += $x1;
				$x1SquareTotal += $x1 * $x1;
				$x1y += $x1 * $y;
				
				$x2Total += $x2;
				$x2SquareTotal += $x2 * $x2;
				$x2y += $x2 * $y;
				
				$yTotal += $y;
				$x1x2 += $x1 * $x2;
            }
			$x1Mean = $x1Total / $N;
			$x2Mean = $x2Total / $N;
			$yMean = $yTotal / $N;
			
			$x1SquareTotal = $x1SquareTotal - $x1Total * $x1Total / $N;
			$x2SquareTotal = $x2SquareTotal - $x2Total * $x2Total / $N;
			
			$x1y = $x1y - $x1Total * $yTotal / $N;
			$x2y = $x2y - $x2Total * $yTotal / $N;
			$x1x2 = $x1x2 - $x1Total * $x2Total / $N;
			
			if(($x1SquareTotal * $x2SquareTotal - $x1x2 * $x1x2)==0)
				$b1 = 0;
			else
				$b1 = ($x2SquareTotal * $x1y - $x1x2 * $x2y) / ($x1SquareTotal * $x2SquareTotal - $x1x2 * $x1x2);
			if(($x1SquareTotal * $x2SquareTotal - $x1x2 * $x1x2)==0)
				$b2 = 0;
			else
				$b2 = ($x1SquareTotal * $x2y - $x1x2 * $x1y) / ($x1SquareTotal * $x2SquareTotal - $x1x2 * $x1x2);
			$b0 = $yMean - $b1 * $x1Mean - $b2 * $x2Mean;
			
			/*$b1 = ($x2SquareTotal * $x1y - $x1x2 * $x2y) / ($x1SquareTotal * $x2SquareTotal - $x1x2 * $x1x2);
			$b2 = ($x1SquareTotal * $x2y - $x1x2 * $x1y) / ($x1SquareTotal * $x2SquareTotal - $x1x2 * $x1x2);
			$b0 = $yMean - $b1 * $x1Mean - $b2 * $x2Mean;*/
			
			$query = "SELECT avg(temp_level) AS average FROM trends ORDER BY trend_id DESC LIMIT 5;";
			$result = mysqli_query($link, $query);
			if(!$result)
				die("Unable to retrieve ".mysqli_error($link));
			$row = mysqli_fetch_assoc($result);
			$x1 = $row['average'];
			
			$query = "SELECT avg(soil_mois_level) AS average FROM trends ORDER BY trend_id DESC LIMIT 5;";
			$result = mysqli_query($link, $query);
			if(!$result)
				die("Unable to retrieve ".mysqli_error($link));
			$row = mysqli_fetch_assoc($result);
			$x2 = $row['average'];
			
			$y = $b0 + $b1 * $x1 + $b2 * $x2;
			$query = "INSERT INTO trends (plant_id,temp_level,soil_mois_level,water_amount,sensors_status,date_time) VALUES ('$plant_id','$x1','$x2','$y','OFF','$date_time');";
			$result = mysqli_query($link, $query);
			if(!$result)
				die("Unable to insert".mysqli_error($link));
			echo json_encode(array("water_amount"=>$y));
        }
    }
	elseif(isset($_POST['CHECK_RECORDS_AVAILABILITY']))
    {
		$query = "SELECT count(*) as total FROM trends;";
        $result = mysqli_query($link, $query);
        if(!$result)
            die("Unable to retrieve ".mysqli_error($link));
		$row = mysqli_fetch_assoc($result);
        if($row['total']==0)
			echo "No Records";
		else
			echo "Records are Available";
    }
	elseif(isset($_POST['CHECK_PLANT_AGE_UPDATE_DATE']))
    {
		$plantID = $_POST['plant_id'];
		date_default_timezone_set("Asia/Muscat");
		$date = date("Y-m-d");

		$query = "select next_update_date as date from plants where plant_id = '$plantID';";
		$result = mysqli_query($link, $query);
		if(!$result)
			die("Unable to insert".mysqli_error($link));
		$row = mysqli_fetch_assoc($result);
		if($date==$row['date'])
		{
			$date = strtotime("360 day", strtotime($date));
			$newDate = date("Y-m-d", $date);
			$query = "update plants set next_update_date = '$newDate', plant_age = plant_age+1 where plant_id = '$plantID';";
			$result = mysqli_query($link, $query);
			if(!$result)
				die("Unable to insert".mysqli_error($link));
			echo "Plant Age Updated";
		}
    }
}
mysqli_close($link);
?>