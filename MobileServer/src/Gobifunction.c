#include "Gobifunction.h"
#include <sys/types.h>
#include <dirent.h>
#include "util.h"
#define MAX_PATH 256

static int charsplit(const char *src, char *desc, int n, const char *splitStr)
{
    int len;

    len = strlen(splitStr);
    const char *p = strstr((const char *)src, (const char *)splitStr);
    if (p == NULL)
        return -1;
    const char *p1 = strstr(p, "\n");
    if (p1 == NULL)
        return -1;
    memset(desc, 0, n);
    memcpy(desc, p + len, p1 - p - len);
    return 0;
}

static int get_dev_major_minor(char *path, int *major, int *minor)
{
    int fd = -1;
    char desc[128] = {0};
    char devmajor[64], devminor[64];
    int n = 0;
    if (access(path, R_OK | W_OK))
    {
        return 1;
    }
    if ((fd = open(path, O_RDWR)) < 0)
    {
        return 1;
    }
    n = read(fd, desc, sizeof(desc));
    if (n == sizeof(desc))
    {
        dbg_time("may be overflow");
    }
    close(fd);
    if (charsplit(desc, devmajor, 64, "MAJOR=") == -1 ||
        charsplit(desc, devminor, 64, "MINOR=") == -1)
    {
        return 2;
    }
    *major = atoi(devmajor);
    *minor = atoi(devminor);
    return 0;
}

int qmidevice_detect(char **pp_qmichannel, char **pp_usbnet_adapter)
{
    struct dirent *ent = NULL;
    DIR *pDir;

    char dir[255] = "/sys/bus/usb/devices";
    int major = 0, minor = 0;
    pDir = opendir(dir);
    if (pDir)
    {
        while ((ent = readdir(pDir)) != NULL)
        {
            struct dirent *subent = NULL;
            DIR *psubDir;
            char subdir[255];
            char subdir2[255 * 2];

            char idVendor[4 + 1] = {0};
            char idProduct[4 + 1] = {0};
            int fd = 0;

            char netcard[32] = "\0";
            char qmifile[32] = "\0";

            //   /sys/bus/usb/devices/3-1.4 idVendor=2c7c idProduct=0125
            //   /sys/bus/usb/devices/3-1.4:1.4/net/eth2
            //   usbnet_adapter = eth2
            //   qmichannel = /dev/qcqmi2
            snprintf(subdir, sizeof(subdir), "%s/%s/idVendor", dir, ent->d_name);
            fd = open(subdir, O_RDONLY);
            if (fd > 0)
            {
                read(fd, idVendor, 4);
                close(fd);
            }

            snprintf(subdir, sizeof(subdir), "%s/%s/idProduct", dir, ent->d_name);
            fd = open(subdir, O_RDONLY);
            if (fd > 0)
            {
                read(fd, idProduct, 4);
                close(fd);
            }

            if (!strncasecmp(idVendor, (char *)"05c6", 4) || !strncasecmp(idVendor, (char *)"2c7c", 4))
                ;
            else
                continue;

            dbg_time("Find %s/%s idVendor=%s idProduct=%s", dir, ent->d_name, idVendor, idProduct);

            snprintf(subdir, sizeof(subdir), "%s/%s:1.4/net", dir, ent->d_name);
            psubDir = opendir(subdir);
            if (psubDir == NULL)
            {
                dbg_time("Cannot open directory: %s, errno: %d (%s)", subdir, errno, strerror(errno));
                continue;
            }

            while ((subent = readdir(psubDir)) != NULL)
            {
                if (subent->d_name[0] == '.')
                    continue;
                dbg_time("Find %s/%s", subdir, subent->d_name);
                dbg_time("Find usbnet_adapter = %s", subent->d_name);
                strcpy(netcard, subent->d_name);
                break;
            }

            closedir(psubDir);

            if (netcard[0])
            {
            }
            else
            {
                continue;
            }

            if (*pp_usbnet_adapter && strcmp(*pp_usbnet_adapter, netcard))
                continue;

            snprintf(subdir, sizeof(subdir), "%s/%s:1.4/GobiQMI", dir, ent->d_name);
            if (access(subdir, R_OK))
            {
                snprintf(subdir, sizeof(subdir), "%s/%s:1.4/usbmisc", dir, ent->d_name);
                if (access(subdir, R_OK))
                {
                    snprintf(subdir, sizeof(subdir), "%s/%s:1.4/usb", dir, ent->d_name);
                    if (access(subdir, R_OK))
                    {
                        dbg_time("no GobiQMI/usbmic/usb found in %s/%s:1.4", dir, ent->d_name);
                        continue;
                    }
                }
            }

            psubDir = opendir(subdir);
            if (psubDir == NULL)
            {
                dbg_time("Cannot open directory: %s, errno: %d (%s)", dir, errno, strerror(errno));
                continue;
            }

            while ((subent = readdir(psubDir)) != NULL)
            {
                if (subent->d_name[0] == '.')
                    continue;
                dbg_time("Find %s/%s", subdir, subent->d_name);
                dbg_time("Find qmichannel = /dev/%s", subent->d_name);
                snprintf(qmifile, sizeof(qmifile), "/dev/%s", subent->d_name);

                //get major minor
                snprintf(subdir2, sizeof(subdir), "%s/%s/uevent", subdir, subent->d_name);
                if (!get_dev_major_minor(subdir2, &major, &minor))
                {
                    dbg_time("%s major = %d, minor = %d\n", qmifile, major, minor);
                }
                else
                {
                    dbg_time("get %s major and minor failed\n", qmifile);
                }
                //get major minor

                if ((fd = open(qmifile, R_OK)) < 0)
                {
                    dbg_time("%s open failed", qmifile);
                    dbg_time("please mknod %s c %d %d", qmifile, major, minor);
                }
                else
                {
                    close(fd);
                }
                break;
            }

            closedir(psubDir);

            if (netcard[0] && qmifile[0])
            {
                *pp_qmichannel = strdup(qmifile);
                *pp_usbnet_adapter = strdup(netcard);
                closedir(pDir);
                return 1;
            }
        }

        closedir(pDir);
    }

    if ((pDir = opendir("/dev")) == NULL)
    {
        dbg_time("Cannot open directory: %s, errno:%d (%s)", "/dev", errno, strerror(errno));
        return -ENODEV;
    }

    while ((ent = readdir(pDir)) != NULL)
    {
        if ((strncmp(ent->d_name, "cdc-wdm", strlen("cdc-wdm")) == 0) || (strncmp(ent->d_name, "qcqmi", strlen("qcqmi")) == 0))
        {
            char net_path[64];

            *pp_qmichannel = (char *)malloc(32);
            sprintf(*pp_qmichannel, "/dev/%s", ent->d_name);
            dbg_time("Find qmichannel = %s", *pp_qmichannel);

            if (strncmp(ent->d_name, "cdc-wdm", strlen("cdc-wdm")) == 0)
                sprintf(net_path, "/sys/class/net/wwan%s", &ent->d_name[strlen("cdc-wdm")]);
            else
            {
                sprintf(net_path, "/sys/class/net/usb%s", &ent->d_name[strlen("qcqmi")]);
                if (access(net_path, R_OK) && errno == ENOENT)
                    sprintf(net_path, "/sys/class/net/eth%s", &ent->d_name[strlen("qcqmi")]);
            }

            if (access(net_path, R_OK) == 0)
            {
                if (*pp_usbnet_adapter && strcmp(*pp_usbnet_adapter, (net_path + strlen("/sys/class/net/"))))
                {
                    free(*pp_qmichannel);
                    *pp_qmichannel = NULL;
                    continue;
                }
                *pp_usbnet_adapter = strdup(net_path + strlen("/sys/class/net/"));
                dbg_time("Find usbnet_adapter = %s", *pp_usbnet_adapter);
                break;
            }
            else
            {
                dbg_time("Failed to access %s, errno:%d (%s)", net_path, errno, strerror(errno));
                free(*pp_qmichannel);
                *pp_qmichannel = NULL;
            }
        }
    }
    closedir(pDir);

    return (*pp_qmichannel && *pp_usbnet_adapter);
}

static int ls_dir(const char *dir, int (*match)(const char *dir, const char *file, void *argv[]), void *argv[])
{
    DIR *pDir;
    struct dirent *ent = NULL;
    int match_times = 0;

    pDir = opendir(dir);
    if (pDir == NULL)
    {
        dbg_time("Cannot open directory: %s, errno: %d (%s)", dir, errno, strerror(errno));
        return 0;
    }

    while ((ent = readdir(pDir)) != NULL)
    {
        match_times += match(dir, ent->d_name, argv);
    }
    closedir(pDir);

    return match_times;
}

static int is_same_linkfile(const char *dir, const char *file, void *argv[])
{
    const char *qmichannel = (const char *)argv[1];
    char linkname[MAX_PATH];
    char filename[MAX_PATH];
    int linksize;

    snprintf(linkname, MAX_PATH, "%s/%s", dir, file);
    linksize = readlink(linkname, filename, MAX_PATH);
    if (linksize <= 0)
        return 0;

    filename[linksize] = 0;
    if (strcmp(filename, qmichannel))
        return 0;

    dbg_time("%s -> %s", linkname, filename);
    return 1;
}

static int is_brother_process(const char *dir, const char *file, void *argv[])
{

    //const char *myself = (const char *)argv[0];
    char linkname[MAX_PATH];
    char filename[MAX_PATH];
    int linksize;
    int i = 0, kill_timeout = 15;
    pid_t pid;

    //dbg_time("%s", file);
    while (file[i])
    {
        if (!isdigit(file[i]))
            break;
        i++;
    }

    if (file[i])
    {
        //dbg_time("%s not digit", file);
        return 0;
    }
    snprintf(linkname, MAX_PATH, "%s/%s/exe", dir, file);
    linksize = readlink(linkname, filename, MAX_PATH);
    if (linksize <= 0)
        return 0;

    filename[linksize] = 0;
#if 0 //check all process
	   if (strcmp(filename, myself))
		   return 0;
#endif

    pid = atoi(file);
    if (pid == getpid())
        return 0;

    snprintf(linkname, MAX_PATH, "%s/%s/fd", dir, file);
    if (!ls_dir(linkname, is_same_linkfile, argv))
        return 0;

    dbg_time("%s/%s/exe -> %s", dir, file, filename);
    while (kill_timeout-- && !kill(pid, 0))
    {
        kill(pid, SIGTERM);
        sleep(1);
    }

    if (!kill(pid, 0))
    {
        dbg_time("force kill %s/%s/exe -> %s", dir, file, filename);
        kill(pid, SIGKILL);
        sleep(1);
    }

    return 1;
}

int kill_brothers(PROFILE_T *profile)
{
    char myself[MAX_PATH];
    int filenamesize;

    if (!profile->qmichannel)
        return -1;

    void *argv[2] = {myself, (void *)profile->qmichannel};

    filenamesize = readlink("/proc/self/exe", myself, MAX_PATH);
    if (filenamesize <= 0)
        return 0;
    myself[filenamesize] = 0;
    if (ls_dir("/proc", is_brother_process, argv))
        sleep(1);
    return 0;
}
