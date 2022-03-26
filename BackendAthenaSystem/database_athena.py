import pymysql
from datetime import datetime

HOST = "127.0.0.1"
USER = "system"
PASSWORD = "SYStem2021!!"
DB_NAME = "AthenaSystem"



class database:

    def __init__(self):
        try:
            self._connection = pymysql.connect(host=HOST, user=USER, password=PASSWORD, database=DB_NAME)
            
        except Exception as exception:
            print(exception)

    def _execute_SQL(self, sql:str):

        try:
            cursor = self._connection.cursor()
            
            cursor.execute(sql)

            self._connection.commit()

            result = cursor.fetchall()
            
            cursor.close()
            
            return result

        except Exception as exception:
            return exception


class db_system(database):

    def __init__(self):
        super().__init__()

    def update_state(self, values:tuple):
        
        self._execute_SQL(f"""UPDATE system SET 
                                manual_auto = \'{values[0]}\',
                                current_position_x = \'{values[1]}\',
                                current_position_y = \'{values[2]}\',
                                current_position_z = \'{values[3]}\',
                                machine_status = \'{values[4]}\',
                                Ima_active = \'{values[5]}\',
                                update_products = \'{values[6]}\';""")


class db_hmi_commands(database):

    def __init__(self):
        super().__init__()
    
    def check_new_commands(self):
        
        result = self._execute_SQL("SELECT * FROM `hmi_commands` LIMIT 0, 1;")

        id = 0
        command = None
        value = None
        
    
        result_is_not_empty = result != ()

        if result_is_not_empty:

            id, command, value = result[0]

            value = eval(value)
       
        return id, command, value

    def delete_executed_command(self, id_command):
        self._execute_SQL(f"DELETE FROM `hmi_commands` WHERE id = {id_command};")
    
    def delete_all_commands(self):
        self._execute_SQL(f"DELETE FROM `hmi_commands`;")


class db_current_products(database):

    def __init__(self):
        super().__init__()

    def add_product_stock(self, value, point):

        product = value[0]
        production_order = value[1]
        quality = value[2]

        entry_datetime = datetime.now()

        self._execute_SQL('INSERT INTO `current_products` (`product`, `production_order`, `quality`, `point`, `entry_datetime`) ' +
                                                 f'VALUES (\"{product}\", \"{production_order}\", \"{quality}\", \"{point}\", \"{entry_datetime}\" );')
       

    def remove_product_id(self, id):

        result = self._execute_SQL(f'SELECT * FROM `current_products` WHERE `id` = {id}')
    
        item = {
                'id'     :result[0][0],
                'product':result[0][1],
                'order'  :result[0][2],
                'quality':result[0][3],
                'point'  :result[0][4],
                'entry'  :result[0][5]
        }

        self._execute_SQL(f"DELETE FROM `current_products` WHERE `id` = {item['id']};")

        return item


    def remove_product_order(self, order):

        result = self._execute_SQL(f'SELECT * FROM `current_products` WHERE `production_order` = {order}')

        item = {
                'id'     :result[0][0],
                'product':result[0][1],
                'order'  :result[0][2],
                'quality':result[0][3],
                'point'  :result[0][4],
                'entry'  :result[0][5]
        }

        self._execute_SQL(f"DELETE FROM `current_products` WHERE `id` = {item['id']};")

        return item


    def delete_all_products(self):
        self._execute_SQL(f"DELETE FROM `current_products`;")


class db_products_history(database):

    def __init__(self):
        super().__init__()

    def add_history_product(self, item):

        exit_datetime = datetime.now()

        print(self._execute_SQL(f"""INSERT INTO `products_history` (`id`, `product`, `production_order`, `quality`, `point`, `entry_datetime`, `exit_datetime`)
                              VALUES (\"{item['id']}\",
                                      \"{item['product']}\", 
                                      \"{item['order']}\",
                                      \"{item['quality']}\",
                                      \"{item['point']}\",
                                      \"{item['entry']}\",
                                      \"{exit_datetime}\"
                                      );"""))
       

    def delete_all_products(self):
        self._execute_SQL(f"DELETE FROM `products_history`;")