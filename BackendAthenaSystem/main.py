from database_athena import *
from machine_athena import Machine
from time import sleep


machine = Machine('COM5')

db_system = db_system()
db_hmi_commands = db_hmi_commands()
db_current_products = db_current_products()
db_products_history = db_products_history()

db_hmi_commands.delete_all_commands()
db_current_products.delete_all_products()

while True:

    id, command, value = db_hmi_commands.check_new_commands()
    
    if id > 0:
        if command == "incrementCoordinate":
            machine.increment_coordinate(value)

        if command == "manualDirectionMovement":
            machine.create_manual_movement(value)
        
        if command == "manualMovementDistance":
            machine.change_manual_movement_distance(value)
        
        if command == "manualAuto":
            machine.set_manual_auto(value)

        if command == "buttonConfirmEntry":
            point_for_add = machine.get_free_point()
            db_current_products.add_product_stock(value, point_for_add)
            machine.insert_in_position(point_for_add)
        
        if command == "currentProductsUpdated":
            machine.current_products_updated()

        if command == "EixoZManual":
            machine.manual_z_axis(value)

        if command == "pickUpAndDrop":
            machine.pick_up(value)
        
        if command == "goToHome":
            machine.go_to_home_position()

        if command == "resetMachine":
            machine.reset_machine()
        
        if command == "goToPosition":
            machine.go_to_position(value)

        if command == "exitProductID":
            item = db_current_products.remove_product_id(value)
            db_products_history.add_history_product(item)
            machine.remove_in_position(item['point'])
            
        if command == "exitProductOrder":
            item = db_current_products.remove_product_order(value)
            db_products_history.add_history_product(item)
            machine.remove_in_position(item['point'])
            
        
        db_hmi_commands.delete_executed_command(id)
    
    machine.get_feed_back_grbl()
    
    db_system.update_state(machine.get_state())  
    
    sleep(0.01)

