//
//  SerialPort.swift
//  ArduinoEncryptionLock
//
//  Created by Zhou Fang on 2019/01/05.
//  Copyright © 2019 Zhou Fang. All rights reserved.
//

import Foundation
import ORSSerial

class SerialPort {
    
    private let serialPort: ORSSerialPort?
    var path: String
    
    init(withPath serialPath: String, andBaudrate baudrate: NSNumber, delegate: ORSSerialPortDelegate) {
        serialPort = ORSSerialPort(path: serialPath)
        path = serialPath
        
        if serialPort == nil {
            print("Error occured in ORSSerialPort initialization.")
        } else {
            serialPort!.delegate = delegate
            serialPort!.baudRate = baudrate
        }
    }
    
    func start() {
        serialPort!.open()
        
    }
    
    func close() {
        serialPort!.close()
    }
    
    func send(withData customText: String) {
        DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
            if let data = customText.data(using: String.Encoding.utf8) {
                self.serialPort!.send(data)
            }
            print(customText)
        }
    }
}
