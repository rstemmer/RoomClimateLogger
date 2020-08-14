<?php
// RoomClimateLogger,  log the climate of all your rooms in a central database.
// Copyright (C) 2020  Ralf Stemmer <ralf.stemmer@gmx.net>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

$DBPATH = "/data/climatelog/climate.db";
main($DBPATH);



function main($dbpath)
{
    $param = GetInput();
    $param["day"]   =   14;
    $param["month"] =    8;
    $param["year"]  = 2020;
    $data  = GetData($dbpath, $param);
    echo json_encode($data);
}



function GetInput()
{
    $date  = getdate();
    $day   = "";
    $month = "";
    $year  = "";

    if($_SERVER["REQUEST_METHOD"] == "GET")
    {
        $day   = filter_input(INPUT_GET, "day",   FILTER_SANITIZE_NUMBER_INT);
        $month = filter_input(INPUT_GET, "month", FILTER_SANITIZE_NUMBER_INT);
        $year  = filter_input(INPUT_GET, "year",  FILTER_SANITIZE_NUMBER_INT);
    }

    if(!isset($day))
        $day = $date["mday"];
    if(!isset($month))
        $month = $date["mon"];
    if(!isset($year))
        $year = $date["year"];

    $retval = array(
            "day"   => $day,
            "month" => $month,
            "year"  => $year
        );

    return $retval;
}



// expects an array with the following keys: "r", "t", "h" for room, temperature and humidity
function GetData($dbpath, $param)
{
    $db  = new SQLite3($dbpath);

    $stm = $db->prepare("SELECT temperature, humidity, room FROM climate WHERE day = :day; AND month = :month AND year = :year");

    $stm->bindValue(":day",    $param["day"],    SQLITE3_INTEGER);
    $stm->bindValue(":month",  $param["month"],  SQLITE3_INTEGER);
    $stm->bindValue(":year",   $param["year"],   SQLITE3_INTEGER);
    
    $result = $stm->execute();
    
    //$data   = $result->fetchArray();
    $data = array();
    $i    = 0;
    while($row = $result->fetchArray(SQLITE3_ASSOC))
    {
        $data[$i] = array();
        $data[$i]["room"]        = $row["room"];
        $data[$i]["temperature"] = $row["temperature"];
        $data[$i]["humidity"]    = $row["humidity"];
        $i++;
    }


    $result->finalize();

    $db->close();

    return $data;
}


?>

