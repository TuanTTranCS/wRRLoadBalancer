# wRRLoadBalancer
Weighted Round Robin Load Balancer for OMNeT++ 5G slicing VNF Infrastructure

This is  a part of my thesis project: A Flexible Heuristic Closed-Loop Algorithm for QoS Assurance in 5G End-to-End Network Slices.


In the OMNeT++ simulation environment, the 5G Core (5GC) allows for the flexible formation of virtual User Plane Functions (UPFs), which can consist of one or more Virtual Network Function (VNF) examples. This feature enables the monitoring of resource usage (such as CPU, RAM, and storage) and permits vertical and horizontal scaling of these instances. We improved our previous simulator's load balancing function with a weighted round-robin mechanism to manage multiple VNFs that serve a network slice.
This mechanism distributes packets to VNF instances based on their relative resource capacity compared to the total capacity of all VNFs serving the slice:

1. Resource limits of serving VNF instances are normalized based on total resource limits of each resource type 
2. The dominant factor of each VNF instance is determined by selecting the maximum value among the 3 normalized resource types. 
3. The VNF's probability to be selected is determined by summing the dominant factors of all VNFs and normalizing the values.