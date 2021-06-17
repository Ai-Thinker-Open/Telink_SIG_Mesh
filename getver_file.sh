VER_FILE=rev-list.txt
VER_REPO_FILE=rev-list-repo.txt
git rev-list --branches | wc -l > $VER_FILE
git rev-list --branches --pretty=format:"%cd %an">> $VER_FILE
perl rev-list.pl $VER_FILE > $VER_REPO_FILE

