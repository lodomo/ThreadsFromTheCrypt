# Notes for development from class
- Password Hash Cracker

-Stat the file first to see how big it is.
-strtok the file to get the hashed password, strtok on new line
-Each thread has a data structure that holds the statistics of the passwords it has tried.
-Global data structure that holds the statistics of all the threads.

- crypt_rn(): built in fragility, might have failure modes
- crypy_ra(): deallocation in a pain in the ass
- crypt_r(): Use this one

Man page lies about the crypt_data

