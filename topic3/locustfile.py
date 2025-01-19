# para executar: locust -f locustfile.py --host=http://127.0.0.1:8080


from locust import HttpUser, task, between

class WebServerUser(HttpUser):
    wait_time = between(1, 2)
    host = "http://127.0.0.1:8080"  

    @task
    def hello_world(self):
        self.client.get("/")