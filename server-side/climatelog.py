#!/usr/bin/env python3

import time
import datetime
import sys
import sqlite3
from sterm.uart import UART

class Sensors(UART):
    def __init__(self, devpath):
        UART.__init__(self, devpath, 9600, "8N1")
        self.shutdown = False



    def Shutdown(self):
        self.shutdown = True



    def GetLine(self):
        line = ""

        while not self.shutdown:
            time.sleep(0.01)
            if self.uart.in_waiting == 0:
                continue

            byte = self.uart.read(1)
            char = byte.decode("utf-8") # I know that only the following values arrive: ['0'..'9'],'.',',','\n','\r'

            if char == "\n":
                return line
            elif char == "\r":
                pass
            else:
                line += char

            time.sleep(0.01)
        return None



    def GetData(self):
        line = self.GetLine()

        try:
            temp,hum = line.split(",")
        except ValueError:
            print("\"%s\" is not a valit temperate,humidity tuple"%(line))
            return None,None

        temp = float(temp)
        hum  = float(hum)

        return temp,hum



    def GetDataUntil(self, deadline):
        t_sum = 0.0
        h_sum = 0.0
        n     = 0

        while(time.time() < deadline):
            t, h = self.GetData()

            if t == None or h == None:
                continue

            t_sum += t
            h_sum += h
            n     += 1

        if n == 0:
            return (None, None, 0)

        return (t_sum / n, h_sum / n, n)


class Database(object):
    def __init__(self, dbpath):
        self.db_connection = sqlite3.connect(dbpath, timeout=20)
        self.db_cursor     = self.db_connection.cursor()

    def Execute(self, sql, values=None):
        try:
            self.db_cursor.execute(sql, values)
        except Exception as e:
            self.db_connection.rollback()
            print(e)

        self.db_connection.commit()
        return None



if __name__ == "__main__":
    devpath = sys.argv[1]
    sensors = Sensors(devpath)
    database= Database("./climate.db")

    deadline = time.time() + 60
    while True:
        temperature, humidity, numsamples = sensors.GetDataUntil(deadline)

        now    = datetime.datetime.now()
        year   = now.year
        month  = now.month
        day    = now.day
        hour   = now.hour
        minute = now.minute

        print(  "\033[1;37m%2i:%2i "
                "\033[0;31mt=\033[1;31m%.2f\033[0;31m°C\033[1;34m, "
                "\033[0;36mh=\033[1;36m%.2f\033[0;36m%%\033[0m"
                %(hour, minute, temperature, humidity))

        sql = "INSERT INTO climate (temperature, humidity, room, day, month, year, hour, minute) VALUES(?,?,?,?,?,?,?,?);"
        values = (temperature, humidity, "Arbeitszimmer", day, month, year, hour, minute)

        database.Execute(sql, values)

        deadline += 60



# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

