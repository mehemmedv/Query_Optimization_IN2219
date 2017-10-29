# Query_Optimization_IN2219

### Exercise 1.1
After `make`, bin/hw01-1example can be used to run and get the result for the following query:

`select s2.name from studenten s1, studenten s2, hoeren h1, hoeren h2 where s1.matrnr = h1.matrnr and s2.matrnr = h2.matrnr and h1.vorlnr = h2.vorlnr and s2.matrnr != s1.matrnr and s1.name = 'Schopenhauer' `

### Exercise 1.2
After `make`, bin/hw01-2example can be used to run and get the result for the following query:

`select distinct p.name from professoren p, vorlesungen v, hoeren h where p.persnr = v.gelesenvon and v.vorlnr = h.vorlnr group by p.name, v.vorlnr having count(*) >= 2`

### Trying out SQL Queries
After `make`, bin/parserexample can be used to run and get the results for any SQL query in the given format such as:

```select titel from vorlesungen v, studenten s, hoeren h where s.matrnr = h.matrnr and v.vorlnr = h.vorlnr and s.semester = 2

select * from studenten s1, studenten s2, hoeren h1, hoeren h2 where s1.matrnr = h1.matrnr and s2.matrnr = h2.matrnr and h1.vorlnr = h2.vorlnr and s1.name = 'Schopenhauer'```
